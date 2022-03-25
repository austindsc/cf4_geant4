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
  
  fGammaStatus = false;
  fGammaHeight = -1. * cm; // NO gamma source
  
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
  
  //--------------------------- Outer Cylinder ---------------------------//
  // outer cylinder (vacuum?)
  // G4double outerCyl_r = 15*cm;
  // G4double outerCyl_thick = 1*cm;
  // G4double outerCyl_z = 100*cm;
  // G4Material* cylSteel = G4Material::GetMaterial("G4_STAINLESS-STEEL");
  // 
  // G4Tubs * solidOuterCyl = new G4Tubs("outerCyl", outerCyl_r, outerCyl_r + outerCyl_thick, 0.5*outerCyl_z, 0, 360.*deg);
  // G4LogicalVolume * logicOuterWorld = new G4LogicalVolume(solidOuterCyl, mWorld)       
  

  //--------------------------- active volume (CF4 Cylinder) ---------------------------//
  // centered on XY place, goes from 0 to innerCyl_z on z axis
  // internal volume of cylinder
  G4double innerCyl_r = 4*cm;
  // G4double innerCyl_t = 1*cm; //thickness
  G4double innerCyl_z = 15*cm;
  
  G4Tubs * solid_medium = new G4Tubs("solid_detectionMedium", 0., innerCyl_r, 0.5*innerCyl_z, 0, 360.*deg);
  G4LogicalVolume * logic_medium = new G4LogicalVolume(solid_medium, *(detMat->fDetectorMedium), "logic_medium");
  logic_medium->SetUserLimits(myLimits);
  G4VPhysicalVolume* phys_medium = new G4PVPlacement(0,                    //no rotation
                    G4ThreeVector(0, 0, 0),      //at (0,0,0)
                    logic_medium,            //its logical volume
                    "phys_detectionMedium",           //its name
                    logicWorld,           //its mother  volume
                    false,                //no boolean operation
                    0,                    //copy number
                    checkOverlaps);       //overlaps checking
  
                    
  //--------------------------- SiPM volume (Box) ---------------------------//
  G4double sipmXY = 15*mm;
  G4double sipmThick = 5*mm;
  G4Box * solid_sipm = new G4Box("solid_sipm", 0.5*sipmXY, 0.5*sipmXY, 0.5*sipmThick);
  // sipmSilicon->GetMaterialTable();
  G4LogicalVolume * logic_sipm = new G4LogicalVolume(solid_sipm, detMat->mSilicon, "logic_sipm");
  G4VPhysicalVolume* phys_sipm = new G4PVPlacement(0, // no rotation
                    G4ThreeVector(0, 0, -sipmThick/2), // SiPM face is at (0, 0, 0)
                    logic_sipm,
                    "phys_sipm",
                    logic_medium,         // mother volume
                    false, 0, checkOverlaps);
                    
  //--------------------------- Gamma Lead Shielding ---------------------------//
  if(fGammaStatus == true){ 
    collimator * logic_coll = new collimator(checkOverlaps);
    G4VPhysicalVolume * phys_collimator = logic_coll->doPlacement(fGammaHeight, logicWorld, checkOverlaps);
  }
  
  //--------------------------- Alpha Source ---------------------------//
  if(fAlphaHeight >= 0 && fAlphaStatus == true) new alphaSource1(logic_medium, fAlphaHeight, checkOverlaps);
  
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