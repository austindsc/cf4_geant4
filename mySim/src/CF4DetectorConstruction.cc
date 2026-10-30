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
#include "G4SDManager.hh"

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
    
    SensitiveSIPM * ssipm = new SensitiveSIPM(DETECTORSETUP::GetSiPMName());
    if(!fsipmVolume) {
        G4cerr << "Error: fsipmVolume is null!" << G4endl;
        exit(1);
    }
    fsipmVolume->SetSensitiveDetector(ssipm);

    G4SDManager* SDman = G4SDManager::GetSDMpointer();
    SDman->AddNewDetector(ssipm);
    
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
  G4LogicalVolume * logic_medium = new G4LogicalVolume(solid_medium, detMat->GetDetectorMedium(), "logic_medium");

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

  // always make the rods stainless
  new G4LogicalSkinSurface("rodSurface", logic_rods, detMat->opSteel_rodSurface);


  //------------- Doing SiPM -------------//
  // put the SiPM surface at the world origin
  G4ThreeVector movePackage = G4ThreeVector(0, 0, -phys_medium->GetTranslation().getZ() - (VUV4::fPackageThick/2 + VUV4::fEdgeBuffer));

  // setup rotation vector (10 ish degrees down), about x axis, so perpendicular to the gamma direction
  G4double rotationAngle = 20.0*degree;
  G4RotationMatrix* rotateSiPM = new G4RotationMatrix();
  rotateSiPM->rotateX(rotationAngle); // some rotation

  G4bool doOptics, buildWindow;
  VUV4 * logic_vuv4 = new VUV4(0, doOptics = true, buildWindow = DETECTORSETUP::GetWindowStatus(), checkOverlaps);
  logic_vuv4->SetVisAttributes(world_transparent);
  G4VPhysicalVolume* phys_vuv4 = new G4PVPlacement(rotateSiPM, movePackage, (G4LogicalVolume*) logic_vuv4, "phys_vuv4", logic_medium, false, 0, checkOverlaps);
  fsipmVolume = logic_vuv4->GetSensitiveVolume(); // VERY IMPORTANT

  //-- building absorptive stuff below (make it plastic covered with absorbing skin)
  G4Tubs * solid_absorber = new G4Tubs("solid_absorber", 0, mediumR * 0.9, 2*mm, 0, 360.*deg);
  G4LogicalVolume * logic_absorber = new G4LogicalVolume(solid_absorber, detMat->mGenericPlastic, "logic_absorber");
  logic_absorber->SetVisAttributes(abs_black);
  G4ThreeVector movePCB = movePackage + G4ThreeVector(0, 0, - (VUV4::fPackageThick/2 + VUV4::fEdgeBuffer) - solid_absorber->GetZHalfLength());
  movePCB.setZ(movePCB.getZ() - std::sin(rotationAngle) * VUV4::fPackageXY); // adjusting for angle
  G4VPhysicalVolume* phys_absorber = new G4PVPlacement(0, movePCB, logic_absorber, "phys_absorber", logic_medium, false, 0, checkOverlaps);
  new G4LogicalSkinSurface("opPCB_absorber", logic_absorber, (G4SurfaceProperty*) detMat->opSurf_perfectAbsorber);
  

/* testing with simple silicon square
  //-- silicon bulk --//
  G4double sipmThick = 1*cm; // guess
  G4double sipmXY = VUV4::fPackageXY;
  G4double sipmSurfaceThick = 1*mm;
  G4double additionalBoxThick = 1*mm;

  G4Box * solid_sipmBulk = new G4Box("solid_sipmBulk", 0.5*sipmXY, 0.5*sipmXY, 0.5*sipmThick);
  G4LogicalVolume * logic_sipmBulk = new G4LogicalVolume(solid_sipmBulk, detMat->mSilicon, "logic_sipmBulk");
  logic_sipmBulk->SetVisAttributes(si_magenta);
  // G4ThreeVector moveSipm = G4ThreeVector(0, 0, 0);
  G4VPhysicalVolume* phys_sipmBulk = new G4PVPlacement(0, movePackage, logic_sipmBulk, "phys_sipmBulk",
                    logic_medium,         // mother volume
                    false, 0, checkOverlaps);
  
  
  //-- silicon surface (Quartz) --//
  G4Box * solid_sipmSurface = new G4Box("solid_sipmSurface", 0.5*sipmXY, 0.5*sipmXY, 0.5*sipmSurfaceThick);
  G4LogicalVolume * logic_sipmSurface = new G4LogicalVolume(solid_sipmSurface, detMat->mQuartz, "logic_sipmSurface");
  logic_sipmSurface->SetVisAttributes(filter_yellow);

  G4ThreeVector moveSurface = movePackage + G4ThreeVector(0, 0, solid_sipmBulk->GetZHalfLength() + solid_sipmSurface->GetZHalfLength()); // sipm wrt origin of solid_BoxPackage is y, z to rotate
  G4VPhysicalVolume* phys_sipmSurface = new G4PVPlacement(0, moveSurface, logic_sipmSurface, "phys_sipmSurface", logic_medium, false, 0, checkOverlaps); //xx
  
  fsipmVolume = logic_sipmBulk; // VERY IMPORTANT

  // ---- Additional Box for correction----//
  G4Box* solid_additionalBox = new G4Box("solid_additionalBox", 0.5 * sipmXY, 0.5 * sipmXY, 0.5 * additionalBoxThick);
  G4LogicalVolume* logic_additionalBox = new G4LogicalVolume(solid_additionalBox, detMat->GetDetectorMedium(), "logic_additionalBox");
  logic_additionalBox->SetVisAttributes(extra_cyan);

  // G4double extraBoxBuffer = 0*mm;
  G4ThreeVector moveAdditionalBox = moveSurface + G4ThreeVector(0, 0, solid_sipmSurface->GetZHalfLength() + solid_additionalBox->GetZHalfLength()); // sipm wrt origin of solid_BoxPackage is y, z);
  G4VPhysicalVolume* phys_additionalBox = new G4PVPlacement(0, moveAdditionalBox, logic_additionalBox, "phys_additionalBox", logic_medium, false, 0, checkOverlaps);


  new G4LogicalSkinSurface("opSurf_siBulk", logic_sipmBulk, (G4SurfaceProperty*) detMat->opSurf_silicon); // silicon bulk, use metal dielectric with Transmission = 100
  
  // doing my PDE correction //xx
  new G4LogicalBorderSurface("opSurf_myCorrector1_in", phys_medium, phys_additionalBox, (G4SurfaceProperty*) detMat->opSurf_myCorrector1_diffuserIn); 
  // // because this is a border, photons going 'backwards' or exiting/reflected off the SiPM just go like normal (they don't see this border).
  new G4LogicalBorderSurface("opSurf_myCorrector2_out", phys_additionalBox, phys_medium, (G4SurfaceProperty*) detMat->opSurf_myCorrector2_absorptionOut);
*/

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
    LongPassFilter * logic_LPFilter = new LongPassFilter();
    // logic_LPFilter->SetupOptics();
    G4ThreeVector moveLPFilter = movePackage + G4ThreeVector(0, 0, (VUV4::fPackageThick/2 + VUV4::fEdgeBuffer)+(LongPassFilter::mTotalThickness/2));

    G4VPhysicalVolume* phys_LPFilter = new G4PVPlacement(rotateSiPM, moveLPFilter, (G4LogicalVolume*) logic_LPFilter, "phys_LPFilter", logic_medium, false, 0, checkOverlaps);
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
  // fsipmVolume = logic_vuv4->GetSensitiveVolume(); // most up to date call (done where buildsipm is)

  //always return the physical World
  return physWorld;
}