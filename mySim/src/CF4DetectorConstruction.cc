#include "G4RunManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Cons.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4Polycone.hh"

#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"

#include "G4Trd.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"

#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"

#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4Element.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4UserLimits.hh"

#include "CF4DetectorConstruction.hh"
#include "CF4DetectorConstructionMessenger.hh"

#include "VUV4.hh"
#include "SensitiveSIPM.hh"
#include "MyGlobals.hh"
#include "DetectorMaterials.hh"
#include "MaterialsMessenger.hh"
#include "collimator.hh"
#include "alphaSource1.hh"
#include "LongPassFilter.hh"
#include "HelperFunctions.hh"

#include <cmath>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CF4DetectorConstruction::CF4DetectorConstruction()
: G4VUserDetectorConstruction(),
  fScoringVolume(0)
{
  fDetMessenger = new CF4DetectorConstructionMessenger(this);
  
  // still control some things with this
  // detMat = new DetectorMaterials();
  detMat = DetectorMaterials::GetInstance();
  MaterialsMessenger * dmatMessenger = new MaterialsMessenger(detMat);
  
  // default values here (if no commands given)
  fAlphaStatus = false;
  fAlphaHeight = -1. * cm; // NO alpha source
  
  fGammaStatus = false; // NO gamma source
  fGammaHeight = -1. * cm;
  
  fLPFilterStatus = false; // no LP filter off the bat

  fCollStatus = true;
  fGammaThetaMax = 2. * deg;
  fGammaSource = "Cs137"; //default, other options are "Co57", etc (see header)

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CF4DetectorConstruction::ConstructSDandField(){
    
    SensitiveSIPM * ssipm = new SensitiveSIPM("SensitiveSIPM");
    fsipmVolume->SetSensitiveDetector(ssipm);
    
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
CF4DetectorConstruction::~CF4DetectorConstruction()
{
  delete fDetMessenger;
  delete detMat;
}

// // extra function to make a quadrant of objects (sipms and their surfaces)
// G4SubtractionSolid* Create4Grid(G4double xy, G4double z, G4double middleGap, G4String solid_name = "solid_squares"){
//   G4double overdoEdges = 1.1;
//   G4Box* si_totalBox = new G4Box("solid_totalBox", 0.5*xy, 0.5*xy, 0.5*z);
//   G4Box* solid_cross1 = new G4Box("solid_cross1", overdoEdges * 0.5*xy, 0.5*middleGap, overdoEdges*z);
//   G4Box* solid_cross2 = new G4Box("solid_cross2", 0.5*middleGap, overdoEdges*0.5*xy, overdoEdges*z);
//   G4UnionSolid* solidCross = new G4UnionSolid("solidCross", solid_cross1, solid_cross2);
//   G4SubtractionSolid* sub_totalBox = new G4SubtractionSolid(solid_name, si_totalBox, solidCross);
//   return sub_totalBox;
// }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* CF4DetectorConstruction::Construct()
{ 
  #include "colours.icc"
  detMat->BuildMaterials(); //PUT THIS HERE! (not in constructor)
  
  G4cout << "---------------------- Parameters in Construct() ----------------------\n" << G4endl;
  G4cout << " alpha height = " << fAlphaHeight/cm << " cm" << G4endl;
  G4cout << "\n--------------------------------------------" << G4endl;

  // Option to switch on/off checking of volumes overlaps
  G4bool checkOverlaps = true;
  
  //--------------------------- World ---------------------------//
  // (lab space where gamma sources live)
  G4double world_sizeXY = 100*cm;
  G4double world_sizeZ  = 100*cm;

  G4Box* solidWorld = new G4Box("World", 0.5*world_sizeXY, 0.5*world_sizeXY, 0.5*world_sizeZ);
  G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, detMat->mVacuum, "World");                   
  
  G4double maxTrackLength = 200*cm;
  G4UserLimits * myLimits = new G4UserLimits();
  myLimits->SetUserMaxTrackLength(maxTrackLength);
  
  logicWorld->SetUserLimits(myLimits);
  G4VPhysicalVolume* physWorld = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0, checkOverlaps);
  
  //--------------------------- CF4 metal Cylinders ---------------------------//
  
  G4double mediumR = 3.48*cm/2;
  G4double mediumZ = 15.*cm;
  G4double packageThick = VUV4::fPackageThick;
  G4double sipmHeightFromBottom = 8.8*cm; // from where to where haha. bottom of cylinder to bottom of SiPM i guess?
  G4ThreeVector moveMediumUP(0, 0, mediumZ/2 - sipmHeightFromBottom - packageThick); //so SiPM face, top of package is still at 0, 0, 0
  
  G4double outerCyl_thick = 0.33*cm;
  
  // upper aluminum of shielding can 1
  G4double can1R = 20.955*cm/2;
  G4double can1Z = 25.4*cm;
  G4double can1thick = 0.635 * cm/2;
  G4double can1PlateZ = 1.27 * cm;
  G4double plateFromTopOfChamber = 1.6*cm;
  G4double upperFlange_solid_thick = 1.2*cm;
  G4ThreeVector tempVector = moveMediumUP + G4ThreeVector(0, 0, mediumZ/2 + upperFlange_solid_thick/2);
  G4ThreeVector can1PlateOffset(-5.38*cm, 0, tempVector.getZ() + plateFromTopOfChamber + can1PlateZ/2);

  // last outer cylinder
  G4double outercanR = 24.79*cm/2;
  G4double outercanThick = 0.61/2 * cm;
  G4double outercanZ = 38.73*cm;
  G4double outercanHeightFromPlate = 9.67*cm;
  
  #include "VacuumVessel.icc";
  
  //--------------------------- active volume (CF4 Cylinder) ---------------------------//
  // centered on XY place, goes from 0 to innerCyl_z on z axis
  // internal volume of cylinder

  G4Tubs * solid_medium = new G4Tubs("solid_detectionMedium", 0., mediumR, 0.5*mediumZ, 0, 360.*deg);
  G4LogicalVolume * logic_medium = new G4LogicalVolume(solid_medium, *(detMat->fDetectorMedium), "logic_medium");

  logic_medium->SetUserLimits(myLimits);
  G4VPhysicalVolume* phys_medium = new G4PVPlacement(0,                    //no rotation
                    moveMediumUP,      //at (0,0,0)
                    logic_medium,            //its logical volume
                    "phys_detectionMedium",           //its name
                    logicWorld,           //its mother  volume
                    false,                //no boolean operation
                    0,                    //copy number
                    checkOverlaps);       //overlaps checking
                    
  //--------------------------- Absorptive paper ---------------------------//
  // G4double paperThick = 0.2*mm;
  // G4Tubs * solid_paper = new G4Tubs("solid_paper", mediumR-2*paperThick, mediumR-paperThick, 0.5*mediumZ, 0, 360.*deg);
  // 
  //--------------------------- Metal Rods ---------------------------//
  G4double rodsR = 3.*mm/2; // guess from image, assume M3, useful webpage is https://www.trfastenings.com/knowledge-base/thread-geometries/metric-coarse-standard
  G4double rodsInnerR = 2.387*mm/2;
  G4double rodsL = mediumZ - 5*mm;
  G4double threadSpace = 0.45*mm; // for M3 threads
  int numThreads = (int)rodsL/threadSpace;
  G4double threadPlanes[numThreads];
  G4double threadRin[numThreads];
  G4double threadRout[numThreads];
  if(MG::GetVerbosity() > 2) G4cout << "thread spacing:" << G4endl;
  for (size_t i = 0; i < numThreads; i++){
    threadPlanes[i] = i * threadSpace - rodsL/2;
    if(MG::GetVerbosity() > 2 ) G4cout << i * threadSpace << G4endl; 
    threadRin[i] = 0;
    if(i%2 == 0){threadRout[i] = rodsR;}
    else{threadRout[i] = rodsInnerR;}
  }
  
  // G4Tubs * solid_rods = new G4Tubs("solid_rods", 0., 1*rodsR, 0.5*rodsL, 0, 360.*deg);
  G4Polycone * solid_rods = new G4Polycone("solid_rods", 0, 360*deg, numThreads, threadPlanes, threadRin, threadRout);
  G4LogicalVolume * logic_rods = new G4LogicalVolume(solid_rods, detMat->mSteel, "logic_rods");
  
  G4double rodPosR = mediumR - rodsR - 1*mm;
  G4ThreeVector rodp1 = G4ThreeVector(rodPosR, 0, -(mediumZ-rodsL)/2);
  new G4PVPlacement(0, rodp1.rotateZ(45*deg), logic_rods, "phys_rod1", logic_medium, false, 0, checkOverlaps);
  new G4PVPlacement(0, rodp1.rotateZ(120*deg), logic_rods, "phys_rod2", logic_medium, false, 0, checkOverlaps);
  new G4PVPlacement(0, rodp1.rotateZ(120*deg), logic_rods, "phys_rod3", logic_medium, false, 0, checkOverlaps);

  new G4LogicalSkinSurface("rodSurface", logic_rods, detMat->opSteel_rodSurface);
  
  //--------------------------- SiPM volumes (Box) ---------------------------//
  // should make sipm like alpha below
  // if(fAlphaHeight >= 0 && fAlphaStatus == true) new alphaSource1(logic_medium, fAlphaHeight - moveMediumUP.getZ(), checkOverlaps);

  // setup rotation vector (10 ish degrees down), about x axis, so perpendicular to the gamma direction
  G4double rotationAngle = 10.0*degree;
  G4RotationMatrix* rotateSiPM = new G4RotationMatrix();
  rotateSiPM->rotateX(rotationAngle); // some rotation

  // silicon quadrants
  G4double sipmMiddleGap = 0.5 * mm; // gap between each quadrant
  G4double sipmXY = 5.9 *  mm; // in reality it is 5.95 x 5.85
  G4double sipmThick = 0.5*mm; // guess
  G4double sipmSurfaceThick = 0.110*um;
  if(DETECTORSETUP::GetVisibleStatus() == true) sipmSurfaceThick = 1*mm;

  // package
  G4double packageXY = 15.*mm;
  // G4double packageThick = 2.5*mm; //defined above
  G4double packageSideGap = 1.5*mm;
  G4double packageSideChannel = 7.*mm;
  // G4double quartzThick = 0.5*mm;
  G4double PackageZGap = 1.3*mm;

  //// doing package first ////
  G4Box * solid_BoxPackage = new G4Box("solid_BoxPackage", 0.5*packageXY, 0.5*packageXY, 0.5*packageThick);
  G4Box * solid_allQuads_toSubtract = new G4Box("solid_allQuads", 0.5*(2*sipmXY + sipmMiddleGap + packageSideGap), 0.5*(2*sipmXY + sipmMiddleGap + packageSideGap), 0.5*(PackageZGap + sipmThick));
  G4Box * solid_rectangleToSubtract = new G4Box("solid_RectangleSubtract", 0.5*(20.*mm), 0.5*packageSideChannel, solid_allQuads_toSubtract->GetZHalfLength()); // this is to add the entry in the side of packaging
  
  G4SubtractionSolid * sub_package = new G4SubtractionSolid("fullBorder_solid_package", solid_BoxPackage, solid_allQuads_toSubtract,
              0, G4ThreeVector(0, 0, solid_BoxPackage->GetZHalfLength() - solid_allQuads_toSubtract->GetZHalfLength())); // this is right
  sub_package = new G4SubtractionSolid("solid_package", sub_package, solid_rectangleToSubtract, 
              0, G4ThreeVector(0, 0, + solid_BoxPackage->GetZHalfLength() - solid_allQuads_toSubtract->GetZHalfLength() + 0.5*sipmThick)); // not sure if + sipmThick is right
  G4LogicalVolume * logic_package = new G4LogicalVolume(sub_package, detMat->mCeramic, "logic_package");
  
  // adding diffusion to ceramic
  new G4LogicalSkinSurface("opSkin_ceramic", logic_package, (G4SurfaceProperty*)detMat->opSurf_ceramic);

  // top of package is at origin!!!
  G4ThreeVector movePackage = G4ThreeVector(0, 0, -phys_medium->GetTranslation().getZ() - solid_BoxPackage->GetZHalfLength());
  G4VPhysicalVolume* phys_package = new G4PVPlacement(rotateSiPM,
                    movePackage,
                    logic_package,
                    "phys_package",
                    logic_medium,
                    false, 0, checkOverlaps);

  // move package puts sipm at center of package  (again, top of package is at origin)
  // logic of moving SiPM (written out bc I keep getting confused)
  // 1. down by half the package (sipm middle at bottom of package)
  // 2. up by half sipm thickness (sipm bottom at bottom of package)
  // 3. up by the difference between boxPackage and subtracted package
  G4double goodZ = 0;
  goodZ = -solid_BoxPackage->GetZHalfLength();
  goodZ += sipmThick/2.;
  goodZ += (packageThick - 2*solid_allQuads_toSubtract->GetZHalfLength());

  // make the Silicon squares
  G4double sipmFullxy = 2*sipmXY + sipmMiddleGap;
  G4double xy, z, middleGap;
  G4String solidName;
  G4SubtractionSolid* solid_siBoxes = VUV4::Create4Grid(xy = sipmFullxy, z = sipmThick, middleGap = sipmMiddleGap, solidName = "solid_siBoxes");

  G4LogicalVolume * logic_si_boxes = new G4LogicalVolume(solid_siBoxes, detMat->mSilicon, "logic_si_boxes");
  logic_si_boxes->SetVisAttributes(si_magenta);

  G4ThreeVector sipmPos_rel_ceramic = movePackage + rotateSiPM->inverse() * G4ThreeVector(0, 0, goodZ);
  G4VPhysicalVolume* phys_si_boxes = new G4PVPlacement(rotateSiPM, sipmPos_rel_ceramic, logic_si_boxes, "phys_si_boxes", logic_medium, false, 0, checkOverlaps);

  // make another cross out of SiO2
  G4SubtractionSolid * solid_passivationBoxes = VUV4::Create4Grid(xy = sipmFullxy, z = sipmSurfaceThick, middleGap = sipmMiddleGap, solidName = "solid_passivationBoxes");
  G4LogicalVolume * logic_passivationBoxes = new G4LogicalVolume(solid_passivationBoxes, detMat->mQuartz, "logic_passivationBoxes");
  logic_passivationBoxes->SetVisAttributes(ssteel_blue);
  G4double surfaceMoveZ = goodZ + sipmThick/2. + sipmSurfaceThick/2.;
  G4ThreeVector moveSipmSurface = movePackage + rotateSiPM->inverse() * G4ThreeVector(0, 0, surfaceMoveZ);
  G4VPhysicalVolume* phys_passivationBoxes = new G4PVPlacement(rotateSiPM, moveSipmSurface, logic_passivationBoxes, "phys_passivationBoxes", logic_medium, false, 0, checkOverlaps);

  //---- adding extra medium above boxes ----//
  G4double extraBox_thick = 1. * um; // very thin box for doing extra optical layers in medium
  if(DETECTORSETUP::GetVisibleStatus() == true)extraBox_thick = 1*mm;

  G4double extraBox_xy = 2*solid_allQuads_toSubtract->GetXHalfLength(); // fill the entire top of SiPM with this (thin) surface
  G4Box* solid_additionalBox = new G4Box("solid_additionalBox", 0.5 * extraBox_xy, 0.5 * extraBox_xy, 0.5 * extraBox_thick);
  G4LogicalVolume* logic_additionalBox = new G4LogicalVolume(solid_additionalBox, detMat->GetDetectorMedium(), "logic_additionalBox");
  logic_additionalBox->SetVisAttributes(world_transparent);
  if(DETECTORSETUP::GetVisibleStatus() == true) logic_additionalBox->SetVisAttributes(filter_yellow);
  
  G4double extraBoxMoveZ = surfaceMoveZ + sipmSurfaceThick/2. + extraBox_thick/2.;
  G4ThreeVector moveAdditionalBox = movePackage + rotateSiPM->inverse() * G4ThreeVector(0, 0, extraBoxMoveZ);
  G4VPhysicalVolume* phys_additionalBox = new G4PVPlacement(rotateSiPM, moveAdditionalBox, logic_additionalBox, "phys_additionalBox", logic_medium, false, 0, checkOverlaps);


  //-------- Doing optical surfaces --------//
  // dielectric_metal of Si bulk
  new G4LogicalSkinSurface("opSurf_siBulk", logic_si_boxes, (G4SurfaceProperty*) detMat->opSurf_silicon); // silicon bulk, use metal dielectric with Transmission = 100
  
  // doing my PDE correction (diffusion in, abs on way out)
  new G4LogicalBorderSurface("opSurf_myCorrector1_in", phys_medium, phys_additionalBox, (G4SurfaceProperty*) detMat->opSurf_myCorrector1_diffuserIn);
  // because this is a border, photons going 'backwards' or exiting/reflected off the SiPM just go like normal (they don't see this border).
  new G4LogicalBorderSurface("opSurf_myCorrector2_out", phys_additionalBox, phys_medium, (G4SurfaceProperty*) detMat->opSurf_myCorrector2_absorptionOut);
  // DONE SIPM

  G4ThreeVector moveTestSipm = movePackage + G4ThreeVector(0, 0, 1*cm);
  G4bool doOptics;
  VUV4 * logic_vuv4 = new VUV4(0, doOptics = true, checkOverlaps);
  logic_vuv4->SetVisAttributes(world_transparent);
  G4VPhysicalVolume* phys_vuv4 = new G4PVPlacement(rotateSiPM, moveTestSipm, (G4LogicalVolume*) logic_vuv4, "phys_vuv4", logic_medium, false, 0, checkOverlaps);
  fsipmVolume = logic_vuv4->GetSensitiveVolume(); // VERY IMPORTANT

  //--------------------------- Longpass filter ---------------------------//      
    /*
    1. define a surface at the top of the LP filter. this is the 'interference coating'
      this should be a 2d material, or atleast very thin
    2. define the optical logic between the detection medium and the surface layer (use skin or border surface)
    3. define same for surface layer and quartz (all transmitted)
    4. define quartz and medium skin interaction (do all transmitted initially)

    // refined
    1. make quartz and medium normal snell's law (don't define surfaces, only n's)
    2. add the top layer to the filter, give it a surface at the top and bottom
    
    3. add some way to kill photons that bounce too many times
    
    // eventually should put all this into a single mother logical volume and then into a longpass filter object

    */

  if(fLPFilterStatus == true){
  
    G4double t = 1.0;
    std::vector<G4double> ephoton = {0.1*eV, 10.0*eV};

    std::vector<G4double> trans = {t, t};
    std::vector<G4double> transPerfect = {1.0, 1.0};

    std::vector<G4double> refPerfect = {0.0, 0.0};
    std::vector<G4double> ref = {1.0 - t, 1.0 - t};

    std::vector<G4double> rindexEphoton = {10.*eV, 7.*eV, 5.*eV, 2.*eV, 1.*eV, 0.1*eV};
    std::vector<G4double> rindex = { 1.24, 1.24, 1.225, 1.21, 1.2039, 1.2039}; // make the same as the medium for full transmission in/out of backside
    rindexEphoton = {10.*eV, 0.1*eV};
    std::vector<G4double> rindexTop = {1.74, 1.74}; // effective index for surface but probably not for glass body
    std::vector<G4double> rindexBody = {1.36, 1.36}; // effective index for body but probably not for surface

    std::vector<G4double> wlsE_filter, transmission_filter, reflectivity_filter; // get out photon energy and transmission
    OH::ReadAndFillArray("dataOptics/lpfilter_500nm.txt", wlsE_filter, transmission_filter, MG::GetVerbosity(), 1., 1./100., true); // scale transmission values by 100, flip wavelengths to eV
    for (std::vector<G4double>::iterator it = transmission_filter.begin(); it != transmission_filter.end(); it++){
      G4double ref = 1.0 - *it;
      reflectivity_filter.push_back(ref);
    }
    // reverse is done in function now
    // std::reverse(wlsE_filter.begin(), wlsE_filter.end());
    // std::reverse(transmission_filter.begin(), transmission_filter.end());
    // std::reverse(reflectivity_filter.begin(), reflectivity_filter.end());

    ////-- defining material properties in one place
    // filter top (interference 'layers')
    G4MaterialPropertiesTable* mptFilterTop = new G4MaterialPropertiesTable();
    mptFilterTop->AddProperty("RINDEX", rindexEphoton, rindexTop);
    G4Material* matTopBoroSilicate = DetectorMaterials::GetInstance()->mBoroSilicate; // do borofloat33 or borosilicate or other metal
    matTopBoroSilicate->SetMaterialPropertiesTable(mptFilterTop);

    // filter body (quartz)
    G4MaterialPropertiesTable* mptFilterBody = new G4MaterialPropertiesTable();
    // mptFilterBody->AddProperty("RINDEX", rindexEphoton, rindexBody);
    mptFilterBody->AddProperty("RINDEX", "Fused Silica"); // built in. only use if body is fused silica
    G4Material * matBodyQuartz = DetectorMaterials::GetInstance()->mQuartz; // quartz
    matBodyQuartz->SetMaterialPropertiesTable(mptFilterBody);

    // G4Material* fAl = new G4Material("Al", z = 13., a = 26.98 * g / mole, density = 2.7 * g / cm3);

    //// ---- Building Physical Filter ----//
    // mother logical (for encapsulating all in it's own LPfilterObject)
    // G4double surfaceThick = 0.1*mm;
    // G4Tubs * motherDisk_filter = new G4Tubs("solid_LPFilter", 0.0, 25.4/2 * mm, 1.1 * mm/2, 0.0, 360.0 * deg);
    // G4LogicalVolume * lp_motherLog = new G4LogicalVolume(motherDisk_filter, detMat->GetDetectorMedium(), "logic_lpMother");

    // LongPassFilter * lp_log = new LongPassFilter();
    G4Tubs * filter_disk = new G4Tubs("solid_LPFilter", 0.0, 25.4/2 * mm, 1.1 * mm/2, 0.0, 360.0 * deg);
    G4LogicalVolume * lp_log = new G4LogicalVolume(filter_disk, matBodyQuartz, "logic_LPFilter");
    lp_log->SetVisAttributes(filter_yellow);

    // would be sexier to use a 'doPlacement' function but oh well
    G4double filterGap = 0.0*cm;
    G4ThreeVector moveFilter = G4ThreeVector(0, 0, movePackage.getZ() + filterGap);
    G4ThreeVector positionFilter = G4ThreeVector(0, 0, solid_BoxPackage->GetZHalfLength() + ((G4Tubs* )lp_log->GetSolid())->GetZHalfLength());
    G4VPhysicalVolume* phys_lpFilter = new G4PVPlacement(rotateSiPM,
                    moveFilter + (rotateSiPM->inverse() *  positionFilter),
                    (G4LogicalVolume *)lp_log,
                    "phys_lpFilter",
                    logic_medium,
                    false, 0, checkOverlaps);

    
    // filter top
    G4Tubs * filterTop_disk = new G4Tubs("filterTop_disk", 0.0, 25.4/2 * mm, 0.1*mm/2, 0.0, 360.0 * deg);
    G4LogicalVolume * lpTop_log = new G4LogicalVolume(filterTop_disk, matTopBoroSilicate, "logic_LPFilterTop");
    lpTop_log->SetVisAttributes(filter_green);
    
    G4ThreeVector positionFilterTop = G4ThreeVector(0, 0, positionFilter.getZ() + filter_disk->GetZHalfLength() + filterTop_disk->GetZHalfLength());
    G4VPhysicalVolume* phys_lpFilterTop = new G4PVPlacement(rotateSiPM,
                    moveFilter + (rotateSiPM->inverse() *  positionFilterTop),
                    (G4LogicalVolume *)lpTop_log,
                    "phys_lpFilterTop",
                    logic_medium,
                    false, 0, checkOverlaps);
    
    ////// ---- doing optical surfaces ---- ////

    // medium <--> surface
    G4MaterialPropertiesTable* mptFilterSurfaceTop = new G4MaterialPropertiesTable();
    mptFilterSurfaceTop->AddProperty("REFLECTIVITY", wlsE_filter, reflectivity_filter);
    mptFilterSurfaceTop->AddProperty("TRANSMITTANCE", wlsE_filter, transmission_filter);
    // mptFilterSurfaceTop->DumpTable();
    // exit(0);

    // surface <--> quartz (body) (PERFECT TRANSMISSION)
    G4MaterialPropertiesTable* mptFilterSurfaceBottom = new G4MaterialPropertiesTable();
    mptFilterSurfaceBottom->AddProperty("REFLECTIVITY", ephoton, refPerfect);
    mptFilterSurfaceBottom->AddProperty("TRANSMITTANCE", ephoton, transPerfect);

    // medium <--> surface  
    G4OpticalSurface* opFilterSurface = new G4OpticalSurface("opFilterSurface");
    opFilterSurface->SetModel(unified);
    opFilterSurface->SetType(dielectric_dielectric);
    opFilterSurface->SetFinish(polishedfrontpainted);
    // opFilterSurface->SetFinish(polished);
    opFilterSurface->SetMaterialPropertiesTable(mptFilterSurfaceTop);
    new G4LogicalBorderSurface("filterSurf", phys_medium, phys_lpFilterTop, opFilterSurface); // do transmission both ways //case A,B,C
    new G4LogicalBorderSurface("filterSurf", phys_lpFilterTop, phys_medium, opFilterSurface); // without this, exiting filter surface is fresnel/rindex //case A

    // surface of filter <--> body of filter
    G4OpticalSurface* opFilterSurfaceBottom = new G4OpticalSurface("opFilterSurfaceBottom");
    opFilterSurfaceBottom->SetType(dielectric_dielectric);
    opFilterSurfaceBottom->SetFinish(polishedfrontpainted);
    opFilterSurfaceBottom->SetModel(unified);
    opFilterSurfaceBottom->SetMaterialPropertiesTable(mptFilterSurfaceBottom);
    new G4LogicalBorderSurface("filterSurf", phys_lpFilterTop, phys_lpFilter, opFilterSurfaceBottom); // comment these out to do fresnel from body to filter top (n are mismatched) // case A, B
    new G4LogicalBorderSurface("filterSurf", phys_lpFilter, phys_lpFilterTop, opFilterSurfaceBottom); // both of these mean perfect transmission in and out //case A, B
  }


  //--------------------------- Collimator ---------------------------//
  // if(fGammaStatus == true)collimator * logic_coll = new collimator(checkOverlaps);
  // can add functionality here to do all this from a modified collimator class
  if(fGammaStatus == true && fCollStatus == true){ // with collimator
    collimator * logic_coll = new collimator(checkOverlaps);
    G4VPhysicalVolume * phys_collimator = logic_coll->doPlacement(fGammaHeight, logicWorld, checkOverlaps);
  }
  else if(fGammaStatus == true && fCollStatus == false){
    //-- actual source 'holder' (this is a dummy item for Gamma sources)
    G4double gsR = 0.4*mm;
    G4double gsZ = 1.*mm;
    
    G4Tubs * solid_gammaSource = new G4Tubs("solid_gammaSource", 0, gsR, gsZ/2, 0, 360.*deg);
    G4LogicalVolume * logic_gammaSource = new G4LogicalVolume(solid_gammaSource, detMat->mGenericPlastic, "logic_gammaSource");
    
    G4RotationMatrix * rot_lead1 = new G4RotationMatrix();
    rot_lead1->rotateY(90*deg);
    G4VPhysicalVolume * phys_gs = new G4PVPlacement(rot_lead1, G4ThreeVector(can1PlateOffset.getX() + outercanR + outercanThick + gsZ, 0, fGammaHeight),
                                  logic_gammaSource, "phys_gammaSource", logicWorld, 0, 0, checkOverlaps);
  }
  
  //--------------------------- Alpha Source ---------------------------//
  if(fAlphaHeight >= 0 && fAlphaStatus == true) new alphaSource1(logic_medium, fAlphaHeight - moveMediumUP.getZ(), checkOverlaps);
  
 //--------------------------- Done Source ---------------------------//

  // Check overlaps in physical volumes
  // G4PhysicalVolumeStore* physicalVolumeStore = G4PhysicalVolumeStore::GetInstance();
  // for (G4VPhysicalVolume* physicalVolume : *physicalVolumeStore) {
  //     if (physicalVolume->CheckOverlaps()) {
  //         G4String volumeName = physicalVolume->GetName();
  //         G4Exception("CheckOverlaps", "GeometryError", FatalException,
  //           G4String("Overlap detected in physical volume " + volumeName).c_str());
  //     }
  // }

  G4cout << "\n---------------------- Done Construct() ----------------------" << G4endl;

  // setting all headers
  fScoringVolume = logic_medium;
  // fsipmVolume = logic_sipm;
  // fsipmVolume = logic_si_boxes;
  fsipmVolume = logic_vuv4->GetSensitiveVolume();

  //always return the physical World
  return physWorld;
}