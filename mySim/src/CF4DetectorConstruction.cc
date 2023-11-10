#include "G4RunManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Cons.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"

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

#include "sensitiveSipm.hh"
#include "myGlobals.hh"
#include "DetectorMaterials.hh"
#include "MaterialsMessenger.hh"
#include "collimator.hh"
#include "alphaSource1.hh"
#include "LongPassFilter.hh"
#include "colours.icc"

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
    
    sensitiveSipm * ssipm = new sensitiveSipm("sensitiveSipm");
    fsipmVolume->SetSensitiveDetector(ssipm);
    
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
CF4DetectorConstruction::~CF4DetectorConstruction()
{
  delete fDetMessenger;
  delete detMat;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* CF4DetectorConstruction::Construct()
{ 
  
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
  G4double packageThick = 2.5*mm;
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
  
  //xx test medium
  // G4Material * fLXe = new G4Material("LXe", 54., 131.29 * g / mole, 3.020 * g / cm3);
  // G4MaterialPropertiesTable* fLXe_mt = new G4MaterialPropertiesTable();

  // std::vector<G4double> lxe_Energy = { 0.1 * eV, 10 * eV };
  // std::vector<G4double> lxe_RIND  = { 1.0, 1.0 };
  // fLXe_mt->AddProperty("RINDEX", lxe_Energy, lxe_RIND);
  // // fLXe_mt->AddProperty("ABSLENGTH", lxe_Energy, lxe_ABSL);
  // fLXe->SetMaterialPropertiesTable(fLXe_mt);
  // //xx done testing

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
  G4double rodsR = 1.2*mm; // guess
  G4double rodsL = mediumZ - 5*mm;
  G4Tubs * solid_rods = new G4Tubs("solid_rods", 0., rodsR, 0.5*rodsL, 0, 360.*deg);
  G4LogicalVolume * logic_rods = new G4LogicalVolume(solid_rods, detMat->mSteel, "logic_rods");
  
  G4double rodPosR = mediumR - rodsR - 1*mm;
  G4ThreeVector rodp1 = G4ThreeVector(rodPosR, 0, -(mediumZ-rodsL)/2);
  new G4PVPlacement(0, rodp1.rotateZ(60*deg), logic_rods, "phys_rod1", logic_medium, false, 0, checkOverlaps);
  new G4PVPlacement(0, rodp1.rotateZ(120*deg), logic_rods, "phys_rod2", logic_medium, false, 0, checkOverlaps);
  new G4PVPlacement(0, rodp1.rotateZ(120*deg), logic_rods, "phys_rod3", logic_medium, false, 0, checkOverlaps);
            
  //--------------------------- SiPM volumes (Box) ---------------------------//
  // should make sipm like alpha below
  // if(fAlphaHeight >= 0 && fAlphaStatus == true) new alphaSource1(logic_medium, fAlphaHeight - moveMediumUP.getZ(), checkOverlaps);

  // silicon quadrants
  G4double sipmMiddleGap = 0.5 * mm; // gap between each quadrant
  G4double sipmXY = 5.9 *  mm; // in reality it is 5.95 x 5.85
  G4double sipmThick = 0.2*mm; // guess

  // package
  G4double packageXY = 15.*mm;
  // G4double packageThick = 2.5*mm; //defined above
  G4double packageSideGap = 1.5*mm;
  G4double packageSideChannel = 7.*mm;
  // G4double quartzThick = 0.5*mm;
  G4double PackageZGap = 1.3*mm;

  // setup rotation vector (10 ish degrees down), about x axis, so perpendicular to the gamma direction
  G4double rotationAngle = 10.0*degree;
  G4RotationMatrix* rotateSiPM = new G4RotationMatrix();
  rotateSiPM->rotateX(rotationAngle); // some rotation

  //// doing package first ////
  G4Box * solid_BoxPackage = new G4Box("solid_BoxPackage", 0.5*packageXY, 0.5*packageXY, 0.5*packageThick);
  G4Box * solid_allQuads_toSubtract = new G4Box("solid_allQuads", 0.5*(2*sipmXY + sipmMiddleGap + packageSideGap), 0.5*(2*sipmXY + sipmMiddleGap + packageSideGap), 0.5*(PackageZGap + sipmThick));
  G4Box * solid_rectangleToSubtract = new G4Box("solid_RectangleSubtract", 0.5*(20.*mm), 0.5*packageSideChannel, solid_allQuads_toSubtract->GetZHalfLength()); // this is to add the entry in the side of packaging
  G4SubtractionSolid * sub_package = new G4SubtractionSolid("fullBorder_solid_package", solid_BoxPackage, solid_allQuads_toSubtract,
              0, G4ThreeVector(0, 0, solid_BoxPackage->GetZHalfLength() - solid_allQuads_toSubtract->GetZHalfLength())); // this is right
  sub_package = new G4SubtractionSolid("solid_package", sub_package, solid_rectangleToSubtract, 
              0, G4ThreeVector(0, 0, + solid_BoxPackage->GetZHalfLength() - solid_allQuads_toSubtract->GetZHalfLength() + 0.5*sipmThick)); // not sure if + sipmThick is right
  G4LogicalVolume * logic_package = new G4LogicalVolume(sub_package, detMat->mCeramic, "logic_package");

  G4ThreeVector movePackage = G4ThreeVector(0, 0, -phys_medium->GetTranslation().getZ() - solid_BoxPackage->GetZHalfLength()); // top of package is at origin
  G4VPhysicalVolume* phys_package = new G4PVPlacement(rotateSiPM,
                    movePackage,
                    logic_package,
                    "phys_package",
                    logic_medium,
                    false, 0, checkOverlaps);


  // silicon quadrants
  // locations
  // G4double sipmSurfaceZ = sipmThick + -2*solid_allQuads_toSubtract->GetZHalfLength() - phys_medium->GetTranslation().getZ(); // SiPM face is at (0, 0, 0) bc translation is within mother volume
  G4double sipmQuad_xyOffset = 0.5 * (sipmMiddleGap + sipmXY);

  G4Box * solid_sipmQuad = new G4Box("solid_sipmQuad", 0.5*sipmXY, 0.5*sipmXY, 0.5*sipmThick);
  G4LogicalVolume * logic_sipm = new G4LogicalVolume(solid_sipmQuad, detMat->mSilicon, "logic_sipm");
  logic_sipm->SetVisAttributes(si_magenta);

  // sipm wrt origin of solid_BoxPackage is y, z to rotate
  // G4double bottomThickness = 2*(solid_BoxPackage->GetZHalfLength() - )
  G4double goodZ = solid_sipmQuad->GetZHalfLength() + solid_BoxPackage->GetZHalfLength() - 2*solid_allQuads_toSubtract->GetZHalfLength();
  // G4ThreeVector firstVec = rotateSiPM->inverse() * G4ThreeVector(sipmQuad_xyOffset, sipmQuad_xyOffset, goodZ);

  // ++ 
  G4VPhysicalVolume* phys_sipm1 = new G4PVPlacement(rotateSiPM, // rotation
                    movePackage + rotateSiPM->inverse() * G4ThreeVector(sipmQuad_xyOffset, sipmQuad_xyOffset, goodZ),
                    logic_sipm,
                    "phys_sipm1",
                    logic_medium,         // mother volume
                    false, 0, checkOverlaps);
  // +-
  G4VPhysicalVolume* phys_sipm2 = new G4PVPlacement(rotateSiPM,
                  movePackage + rotateSiPM->inverse() * G4ThreeVector(sipmQuad_xyOffset, -sipmQuad_xyOffset, goodZ),
                  logic_sipm,
                  "phys_sipm2",
                  logic_medium,         // mother volume
                  false, 0, checkOverlaps);
  
  // --
  G4VPhysicalVolume* phys_sipm3 = new G4PVPlacement(rotateSiPM,
                  movePackage + rotateSiPM->inverse() * G4ThreeVector(-sipmQuad_xyOffset, -sipmQuad_xyOffset, goodZ),
                  logic_sipm,
                  "phys_sipm3",
                  logic_medium,         // mother volume
                  false, 0, checkOverlaps);
  
  // -+
  G4VPhysicalVolume* phys_sipm4 = new G4PVPlacement(rotateSiPM,
                  movePackage + rotateSiPM->inverse() * G4ThreeVector(-sipmQuad_xyOffset, sipmQuad_xyOffset, goodZ),
                  logic_sipm,
                  "phys_sipm4",
                  logic_medium,         // mother volume
                  false, 0, checkOverlaps);
  
 
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


    */
  if(fLPFilterStatus == true){
  
    G4double t = 0.5;
    std::vector<G4double> ephoton = {0.1*eV, 10.0*eV};

    std::vector<G4double> trans = {t, t};
    std::vector<G4double> transPerfect = {1.0, 1.0};

    std::vector<G4double> refPerfect = {0.0, 0.0};
    std::vector<G4double> ref = {1.0 - t, 1.0 - t};

    std::vector<G4double> rindexEphoton = {10.*eV, 7.*eV, 5.*eV, 2.*eV, 1.*eV, 0.1*eV};
    std::vector<G4double> rindex = { 1.24, 1.24, 1.225, 1.21, 1.2039, 1.2039}; // make the same as the medium for full transmission in/out of backside
    // rindex = {1.22, 1.22}; //same as cf4
    // std::vector<G4double> abslength = { 1.*m, 1.*m};

    ////-- defining material properties in one place
    // quartz material
    G4MaterialPropertiesTable* mptFilterBody = new G4MaterialPropertiesTable();
    mptFilterBody->AddProperty("RINDEX", rindexEphoton, rindex);
    G4Material* matQuartz = DetectorMaterials::GetInstance()->mQuartz;
    matQuartz->SetMaterialPropertiesTable(mptFilterBody);

    // medium <--> surface
    G4MaterialPropertiesTable* mptFilterSurfaceTop = new G4MaterialPropertiesTable();
    mptFilterSurfaceTop->AddProperty("REFLECTIVITY", ephoton, ref);
    mptFilterSurfaceTop->AddProperty("TRANSMITTANCE", ephoton, trans);

    // surface <--> quartz (body)
    G4MaterialPropertiesTable* mptFilterSurfaceBottom = new G4MaterialPropertiesTable();
    mptFilterSurfaceBottom->AddProperty("REFLECTIVITY", ephoton, refPerfect);
    mptFilterSurfaceBottom->AddProperty("TRANSMITTANCE", ephoton, transPerfect);

    //-- filter top to filter body
    // body <--> medium , don't need it because have matching indexes of refraction rn
    // G4MaterialPropertiesTable * mptFilterTopSurface = new G4MaterialPropertiesTable();
    // // mptFilterTopSurface->SetNewProperty(true);
    // // mptFilterTopSurface->AddProperty("RINDEX", ephoton, rindex);
    // // mptFilterTopSurface->AddProperty("ABSLENGTH", ephoton, abslength);
    // mptFilterTopSurface->AddProperty("REFLECTIVITY", ephoton, ref);
    // // mptFilterTopSurface->AddProperty("EFFICIENCY", ephoton, efficiency);
    // mptFilterTopSurface->AddProperty("TRANSMITTANCE", ephoton, trans);

    // G4Material* fAl = new G4Material("Al", z = 13., a = 26.98 * g / mole, density = 2.7 * g / cm3);
    // everything is quartz with same index as CF4


    //// ---- Building Physical Filter ----//
    // LongPassFilter * lp_log = new LongPassFilter();
    G4Tubs * filter_disk = new G4Tubs("solid_LPFilter", 0.0, 25.4/2 * mm, 1.1 * mm/2, 0.0, 360.0 * deg);
    G4LogicalVolume * lp_log = new G4LogicalVolume(filter_disk, matQuartz, "logic_LPFilter");
    lp_log->SetVisAttributes(filter_yellow);

    // would be sexier to use a 'doPlacement' function but oh well
    G4ThreeVector moveFilter = G4ThreeVector(0, 0, movePackage.getZ() + 1.*cm);
    G4ThreeVector positionFilter = G4ThreeVector(0, 0, solid_BoxPackage->GetZHalfLength() + ((G4Tubs* )lp_log->GetSolid())->GetZHalfLength());
    G4VPhysicalVolume* phys_lpFilter = new G4PVPlacement(rotateSiPM,
                    moveFilter + (rotateSiPM->inverse() *  positionFilter),
                    (G4LogicalVolume *)lp_log,
                    "phys_lpFilter",
                    logic_medium,
                    false, 0, checkOverlaps);
    
    // filter top
    G4Tubs * filterTop_disk = new G4Tubs("filterTop_disk", 0.0, 25.4/2 * mm, 1.1 * mm/2, 0.0, 360.0 * deg);
    G4LogicalVolume * lpTop_log = new G4LogicalVolume(filter_disk, matQuartz, "logic_LPFilterTop");
    lpTop_log->SetVisAttributes(filter_green);
    
    G4ThreeVector positionFilterTop = G4ThreeVector(0, 0, positionFilter.getZ() + filter_disk->GetZHalfLength() + filterTop_disk->GetZHalfLength());
    G4VPhysicalVolume* phys_lpFilterTop = new G4PVPlacement(rotateSiPM,
                    moveFilter + (rotateSiPM->inverse() *  positionFilterTop),
                    (G4LogicalVolume *)lpTop_log,
                    "phys_lpFilterTop",
                    logic_medium,
                    false, 0, checkOverlaps);
    

    // medium <--> surface  
    G4OpticalSurface* opFilterSurface = new G4OpticalSurface("opFilterSurface"); //, unified, polished, dielectric_metal); // works with dielectric_metal
    opFilterSurface->SetType(dielectric_dielectric);
    opFilterSurface->SetFinish(polishedfrontpainted);
    opFilterSurface->SetModel(unified);
    opFilterSurface->SetMaterialPropertiesTable(mptFilterSurfaceTop);
    new G4LogicalBorderSurface("filterSurf", phys_medium, phys_lpFilterTop, opFilterSurface);

    // medium <--> surface
    G4OpticalSurface* opFilterSurfaceBottom = new G4OpticalSurface("opFilterSurfaceBottom"); //, unified, polished, dielectric_metal); // works with dielectric_metal
    opFilterSurfaceBottom->SetType(dielectric_dielectric);
    opFilterSurfaceBottom->SetFinish(polishedfrontpainted);
    opFilterSurfaceBottom->SetModel(unified);
    opFilterSurfaceBottom->SetMaterialPropertiesTable(mptFilterSurfaceBottom);
    new G4LogicalBorderSurface("filterSurf", phys_lpFilterTop, phys_lpFilter, opFilterSurfaceBottom);

    // // new G4LogicalSkinSurface("filterSurf", lp_log, opFilterSurface);
    // // new G4LogicalBorderSurface("filterSurf", phys_medium, phys_lpFilter, opFilterSurface);

    // two questions: do i need the top layer, and what if i change the refractive indexes to quartz values?

    // then there is inputting PDE and the angular dependance

    
  }

  
  // setting all headers
  fScoringVolume = logic_medium;
  fsipmVolume = logic_sipm;

  // 
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
  G4PhysicalVolumeStore* physicalVolumeStore = G4PhysicalVolumeStore::GetInstance();
  for (G4VPhysicalVolume* physicalVolume : *physicalVolumeStore) {
      if (physicalVolume->CheckOverlaps()) {
          G4String volumeName = physicalVolume->GetName();
          G4Exception("CheckOverlaps", "GeometryError", FatalException,
            G4String("Overlap detected in physical volume " + volumeName).c_str());
      }
  }

  G4cout << "\n---------------------- Done Construct() ----------------------" << G4endl;


  //always return the physical World
  return physWorld;
}