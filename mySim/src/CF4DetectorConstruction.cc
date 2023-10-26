#include "CF4DetectorConstruction.hh"
#include "CF4DetectorConstructionMessenger.hh"
#include "sensitiveSipm.hh"

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

#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4Element.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4UserLimits.hh"

#include "myGlobals.hh"
#include "DetectorMaterials.hh"
#include "MaterialsMessenger.hh"
#include "collimator.hh"
#include "alphaSource1.hh"

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
  G4double sipmHeightFromBottom = 8.8*cm;
  G4ThreeVector moveMediumUP(0, 0, mediumZ/2 - sipmHeightFromBottom); //so SiPM face is still at 0, 0, 0
  
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
  // if(fAlphaHeight >= 0 && fAlphaStatus == true) new alphaSource1(logic_medium, fAlphaHeight - moveMediumUP.getZ(), checkOverlaps);

  // doing quadrants
  G4double sipmMiddleGap = 0.5 * mm; // gap between each quadrant
  G4double sipmXY = 5.9 *  mm; // in reality it is 5.95 x 5.85
  G4double sipmThick = 0.2*mm; // guess
  G4double sipmSurfaceZ = -sipmThick/2 -moveMediumUP.getZ(); // SiPM face is at (0, 0, 0)

  G4double sipmQuad_xyOffset = 0.5 * (sipmMiddleGap + sipmXY);

  // setup rotation vector (10 ish degrees down)

  G4Box * solid_sipmQuad = new G4Box("solid_sipmQuad", 0.5*sipmXY, 0.5*sipmXY, 0.5*sipmThick);

  G4LogicalVolume * logic_sipm = new G4LogicalVolume(solid_sipmQuad, detMat->mSilicon, "logic_sipm");
  logic_sipm->SetVisAttributes(si_magenta);

  // ++ 
  G4VPhysicalVolume* phys_sipm1 = new G4PVPlacement(0, // no rotation
                    G4ThreeVector(sipmQuad_xyOffset, sipmQuad_xyOffset, sipmSurfaceZ),
                    logic_sipm,
                    "phys_sipm1",
                    logic_medium,         // mother volume
                    false, 0, checkOverlaps);
  // +-
  G4VPhysicalVolume* phys_sipm2 = new G4PVPlacement(0, // no rotation
                  G4ThreeVector(sipmQuad_xyOffset, -sipmQuad_xyOffset, sipmSurfaceZ),
                  logic_sipm,
                  "phys_sipm2",
                  logic_medium,         // mother volume
                  false, 0, checkOverlaps);
  
  // --
  G4VPhysicalVolume* phys_sipm3 = new G4PVPlacement(0, // no rotation
                  G4ThreeVector(-sipmQuad_xyOffset, -sipmQuad_xyOffset, sipmSurfaceZ),
                  logic_sipm,
                  "phys_sipm3",
                  logic_medium,         // mother volume
                  false, 0, checkOverlaps);
  
  // -+
  G4VPhysicalVolume* phys_sipm4 = new G4PVPlacement(0, // no rotation
                  G4ThreeVector(-sipmQuad_xyOffset, sipmQuad_xyOffset, sipmSurfaceZ),
                  logic_sipm,
                  "phys_sipm4",
                  logic_medium,         // mother volume
                  false, 0, checkOverlaps);
  
  G4double packageXY = 15.*mm;
  G4double packageThick = 2.5*mm;
  G4double packageSideGap = 1.5*mm;
  // G4double quartzThick = 0.5*mm;
  G4double PackageZGap = 1.3*mm;
  
  // G4Box * solid_subSiPM = new G4Box("solid_subSiPM", 0.5*(2*sipmXY + sipmMiddleGap + packageSideGap), 0.5*(2*sipmXY + sipmMiddleGap + packageSideGap), 0.5*(PackageZGap + sipmThick)); // double tall
  G4Box * solid_BoxPackage = new G4Box("solid_BoxPackage", 0.5*packageXY, 0.5*packageXY, 0.5*packageThick);
  G4Box * solid_allQuads_toSubtract = new G4Box("solid_allQuads", 0.5*(2*sipmXY + sipmMiddleGap + packageSideGap), 0.5*(2*sipmXY + sipmMiddleGap + packageSideGap), 0.5*(PackageZGap + sipmThick));
  G4Box * solid_rectangleToSubtract = new G4Box("solid_RectangleSubtract", 0.5*(20.*mm), 0.5*(7.*mm), solid_allQuads_toSubtract->GetZHalfLength()); // this is to add the entry in the side of packaging

  G4SubtractionSolid * sub_package = new G4SubtractionSolid("fullBorder_solid_package", solid_BoxPackage, solid_allQuads_toSubtract,
              0, G4ThreeVector(0, 0, solid_BoxPackage->GetZHalfLength() - solid_allQuads_toSubtract->GetZHalfLength())); // this is right

  sub_package = new G4SubtractionSolid("solid_package", sub_package, solid_rectangleToSubtract, 
              0, G4ThreeVector(0, 0, + solid_BoxPackage->GetZHalfLength() - solid_allQuads_toSubtract->GetZHalfLength() + 0.5*sipmThick)); // not sure if + sipmThick is right

  G4LogicalVolume * logic_package = new G4LogicalVolume(sub_package, detMat->mCeramic, "logic_package");

  G4ThreeVector temp = G4ThreeVector(0, 0, phys_sipm1->GetTranslation().z() + -sipmThick/2.-packageThick/2. + 0.5*(PackageZGap + sipmThick));
  G4ThreeVector temp2 = G4ThreeVector(0, 0, phys_sipm1->GetTranslation().z() - solid_BoxPackage->GetZHalfLength() - solid_sipmQuad->GetZHalfLength() + 2*solid_allQuads_toSubtract->GetZHalfLength());
  new G4PVPlacement(0,
                    temp2,
                    logic_package,
                    "phys_package",
                    logic_medium,
                    false, 0, checkOverlaps);
  
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


  G4cout << "\n---------------------- Done Construct() ----------------------" << G4endl;

  //always return the physical World
  return physWorld;
}