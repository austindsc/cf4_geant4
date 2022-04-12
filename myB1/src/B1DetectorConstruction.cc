#include "B1DetectorConstruction.hh"
#include "B1DetectorConstructionMessenger.hh"
#include "sensitiveSipm.hh"

#include "G4RunManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Cons.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4Trd.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"

#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

#include "G4Element.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4UserLimits.hh"

#include "DetectorMaterials.hh"
#include "MaterialsMessenger.hh"
#include "collimator.hh"
#include "alphaSource1.hh"

#include <cmath>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1DetectorConstruction::B1DetectorConstruction()
: G4VUserDetectorConstruction(),
  fScoringVolume(0)
{
  fDetMessenger = new B1DetectorConstructionMessenger(this);
  
  // still control some things with this
  // detMat = new DetectorMaterials();
  detMat = DetectorMaterials::GetInstance();
  MaterialsMessenger * dmatMessenger = new MaterialsMessenger(detMat);
  
  // default values here (if no commands given)
  fAlphaStatus = false;
  fAlphaHeight = -1. * cm; // NO alpha source
  
  fGammaStatus = false; // NO gamma source
  fGammaHeight = -1. * cm;
  
  fGammaSource = "Cs137"; //default, other options are "Co60", etc (see header)
  
  // do messenger stuff to set variables in detMat
  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
B1DetectorConstruction::~B1DetectorConstruction()
{
  delete fDetMessenger;
  delete detMat;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* B1DetectorConstruction::Construct()
{ 
  detMat->BuildMaterials(); //PUT THIS HERE! (not in consturctor)
  
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
  
  //--------------------------- CF4 metal Cylinder ---------------------------//
  G4double mediumR = 3.48*cm/2;
  G4double mediumZ = 15*cm;
  G4double sipmHeightFromBottom = 8.8*cm;
  G4double sipmThick = 5*mm;
  G4ThreeVector moveMediumUP(0, 0, mediumZ/2 - sipmHeightFromBottom); //so SiPM face is still at 0, 0, 0
  //G4ThreeVector moveMediumUP(0, 0, 0); //so SiPM face is still at 0, 0, 0
  
  G4double outerCyl_thick = 0.33*cm;
  
  G4Tubs * solid_mediumCyl = new G4Tubs("solid_mediumCyl", mediumR, mediumR + outerCyl_thick, 0.5*mediumZ, 0, 360.*deg);
  G4LogicalVolume * logic_mediumCyl = new G4LogicalVolume(solid_mediumCyl, detMat->mSteel, "logic_mediumCyl");
  new G4PVPlacement(0, moveMediumUP, logic_mediumCyl, "phys_mediumCyl", logicWorld, false, 0, checkOverlaps);
  
  // upper flanges of CF4 cylinder
  G4double flange_r = 6.9*cm/2;
  G4double upperFlange_thick = 1.3*cm;
  G4Tubs * solid_med_upperFlange = new G4Tubs("solid_med_upperFlange", mediumR + outerCyl_thick, flange_r, 0.5*upperFlange_thick, 0, 360*deg);
  G4LogicalVolume * logic_med_upperFlange = new G4LogicalVolume(solid_med_upperFlange, detMat->mSteel, "logic_med_upperFlange");
  new G4PVPlacement(0, moveMediumUP + G4ThreeVector(0, 0, mediumZ/2 - upperFlange_thick/2), logic_med_upperFlange, "phys_med_upperFlange", logicWorld, false, 0, checkOverlaps);

  // solid upper flange
  G4double upperFlange_solid_thick = 1.2*cm;
  G4Tubs * solid_med_solidupperFlange = new G4Tubs("solid_med_solidupperFlange", 0, flange_r, 0.5*upperFlange_solid_thick, 0, 360*deg);
  G4LogicalVolume * logic_med_solidupperFlange = new G4LogicalVolume(solid_med_solidupperFlange, detMat->mSteel, "logic_med_solidupperFlange");
  new G4PVPlacement(0, moveMediumUP + G4ThreeVector(0, 0, mediumZ/2 + upperFlange_solid_thick/2), logic_med_solidupperFlange, "phys_med_solidupperFlange", logicWorld, false, 0, checkOverlaps);

  // lower flanges of CF4 cylinder
  G4double lowerFlange_thick = 3.8*cm;
  G4Tubs * solid_med_lowerFlange = new G4Tubs("solid_med_lowerFlange", mediumR + outerCyl_thick, flange_r, 0.5*lowerFlange_thick, 0, 360*deg);
  G4LogicalVolume * logic_med_lowerFlange = new G4LogicalVolume(solid_med_lowerFlange, detMat->mSteel, "logic_med_lowerFlange");
  new G4PVPlacement(0, moveMediumUP + G4ThreeVector(0, 0, -mediumZ/2 + lowerFlange_thick/2), logic_med_lowerFlange, "phys_med_lowerFlange", logicWorld, false, 0, checkOverlaps);

  // solid lower flange
  new G4PVPlacement(0, moveMediumUP + G4ThreeVector(0, 0, -mediumZ/2 - upperFlange_solid_thick/2), logic_med_solidupperFlange, "phys_med_solidlowerFlange", logicWorld, false, 0, checkOverlaps);

  // upper aluminum of shielding can 1
  G4double can1R = 20.955*cm/2;
  G4double can1Z = 25.4*cm;
  G4double can1thick = 0.635 * cm/2;
  G4double can1PlateZ = 1.27 * cm;
  G4double plateFromTopOfChamber = 1.6*cm;
  G4ThreeVector tempVector = moveMediumUP + G4ThreeVector(0, 0, mediumZ/2 + upperFlange_solid_thick/2);
  G4ThreeVector can1PlateOffset(-5.38*cm, 0, tempVector.getZ() + plateFromTopOfChamber + can1PlateZ/2);
  
  G4Tubs * solid_can1Plate = new G4Tubs("solid_can1Plate", 0, can1R, can1PlateZ/2, 0., 360.*deg);
  G4LogicalVolume * logic_can1Plate = new G4LogicalVolume(solid_can1Plate, detMat->mAluminum, "logic_can1Plate");
  new G4PVPlacement(0, can1PlateOffset, logic_can1Plate, "phys_can1Plate", logicWorld, false, 0, checkOverlaps);
  
  G4Tubs * solid_can1 = new G4Tubs("solid_can1", can1R, can1R+can1thick, can1Z/2, 0., 360.*deg);
  G4LogicalVolume * logic_can1 = new G4LogicalVolume(solid_can1, detMat->mSteel, "logic_can1");
  new G4PVPlacement(0, can1PlateOffset + G4ThreeVector(0, 0, -can1Z/2 + can1PlateZ/2), logic_can1, "phys_can1", logicWorld, false, 0, checkOverlaps);
  
  G4Tubs * solid_can1_bottom = new G4Tubs("solid_can1_bottom", 0, can1R, can1thick/2, 0., 360.*deg);
  G4LogicalVolume * logic_can1_bottom = new G4LogicalVolume(solid_can1_bottom, detMat->mSteel, "logic_can1_bottom");
  new G4PVPlacement(0, can1PlateOffset + G4ThreeVector(0, 0, -can1Z + can1PlateZ/2 + can1thick/2), logic_can1_bottom, "phys_can1_bottom", logicWorld, false, 0, checkOverlaps);

  // last outer cylinder
  G4double outercanR = 24.79*cm/2;
  G4double outercanThick = 0.61/2 * cm;
  G4double outercanZ = 38.73*cm;
  G4double outercanHeightFromPlate = 9.67*cm;
  G4Tubs * solid_outercan = new G4Tubs("solid_outercan", outercanR, outercanR+outercanThick, outercanZ/2, 0, 360.*deg);
  G4LogicalVolume * logic_outercan = new G4LogicalVolume(solid_outercan, detMat->mSteel, "logic_outercan");
  new G4PVPlacement(0, can1PlateOffset + G4ThreeVector(0, 0, -outercanZ/2 + outercanHeightFromPlate), logic_outercan, "phys_outercan", logicWorld, false, 0, checkOverlaps);

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
  G4double paperThick = 0.2*mm;
  G4Tubs * solid_paper = new G4Tubs("solid_paper", mediumR-2*paperThick, mediumR-paperThick, 0.5*mediumZ, 0, 360.*deg);
                    
  //--------------------------- Metal Rods ---------------------------//
  G4double rodsR = 1.5*mm;
  G4double rodsL = mediumZ - 5*mm;
  G4Tubs * solid_rods = new G4Tubs("solid_rods", 0., rodsR, 0.5*rodsL, 0, 360.*deg);
  G4LogicalVolume * logic_rods = new G4LogicalVolume(solid_rods, detMat->mSteel, "logic_rods");
  
  G4double rodPosR = mediumR - rodsR - 1*mm;
  G4ThreeVector rodp1 = G4ThreeVector(rodPosR, 0, -(mediumZ-rodsL)/2);
  new G4PVPlacement(0, rodp1, logic_rods, "phys_rod1", logic_medium, false, 0, checkOverlaps);
  new G4PVPlacement(0, rodp1.rotateZ(120*deg), logic_rods, "phys_rod2", logic_medium, false, 0, checkOverlaps);
  new G4PVPlacement(0, rodp1.rotateZ(120*deg), logic_rods, "phys_rod3", logic_medium, false, 0, checkOverlaps);
            
  //--------------------------- SiPM volume (Box) ---------------------------//
  G4double sipmXY = 15*mm;
  G4Box * solid_sipm = new G4Box("solid_sipm", 0.5*sipmXY, 0.5*sipmXY, 0.5*sipmThick);
  // sipmSilicon->GetMaterialTable();
  G4LogicalVolume * logic_sipm = new G4LogicalVolume(solid_sipm, detMat->mSilicon, "logic_sipm");
  G4VPhysicalVolume* phys_sipm = new G4PVPlacement(0, // no rotation
                    G4ThreeVector(0, 0, -sipmThick/2 -moveMediumUP.getZ()), // SiPM face is at (0, 0, 0)
                    logic_sipm,
                    "phys_sipm",
                    logic_medium,         // mother volume
                    false, 0, checkOverlaps);
                    
  //--------------------------- Collimator ---------------------------//
  if(fGammaStatus == true){ 
    collimator * logic_coll = new collimator(checkOverlaps);
    G4VPhysicalVolume * phys_collimator = logic_coll->doPlacement(fGammaHeight, logicWorld, checkOverlaps);
  }
  
  //--------------------------- Alpha Source ---------------------------//
  if(fAlphaHeight >= 0 && fAlphaStatus == true) new alphaSource1(logic_medium, fAlphaHeight - moveMediumUP.getZ(), checkOverlaps);
  
 //--------------------------- Done Source ---------------------------//
  
  // setting all headers
  fScoringVolume = logic_medium;
  fsipmVolume = logic_sipm;

  G4cout << "\n---------------------- Done Construct() ----------------------" << G4endl;

  //always return the physical World
  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B1DetectorConstruction::ConstructSDandField(){
    
    sensitiveSipm * ssipm = new sensitiveSipm("sensitiveSipm");
    fsipmVolume->SetSensitiveDetector(ssipm);
    
}