//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file B1DetectorConstruction.cc
/// \brief Implementation of the B1DetectorConstruction class


/* Austin here

1. start simple. just a world volume and CF4 cylinder


*/

#include "B1DetectorConstruction.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Cons.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4Trd.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"

#include "G4Element.hh"
#include "G4PhysicalVolumeStore.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1DetectorConstruction::B1DetectorConstruction()
: G4VUserDetectorConstruction(),
  fScoringVolume(0)
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1DetectorConstruction::~B1DetectorConstruction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* B1DetectorConstruction::Construct()
{ 
  // important position variables // 
  G4double sourceToSipm = 5. * cm;
  
  
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();

  // Option to switch on/off checking of volumes overlaps
  G4bool checkOverlaps = true;
  
  ////////////////////------------ all materials ------------//////////////////// 

  // ---------- Vacuum (world) ----------//
  // do dispersion function and fill arrays in loop
  G4double photonEWorld[2] = {1239.841939*eV/140, 1239.841939*eV/1200};
  G4double nWorld[2] = {1.00, 1.00};   
  G4MaterialPropertiesTable *mptWorld = new G4MaterialPropertiesTable();
  mptWorld->AddProperty("RINDEX", photonEWorld, nWorld, 2); //2 is for length of arrays
  
  G4Material* mWorld = nist->FindOrBuildMaterial("G4_Galactic");
  mWorld->SetMaterialPropertiesTable(mptWorld);
  
  // ---------- CF4 ----------//
  G4Element * elCarbon = new G4Element("Carbon", "C", 6, 12.01 * g/mole);
  G4Element * elFluorine = new G4Element("Fluorine", "F", 9,  19. * g/mole);
  
  G4double densityCF4 = 0.002*g/cm3; // slightly denser than STP air

  G4Material * cf4Gas = new G4Material("cf4Gas", densityCF4, 2);
  cf4Gas->AddElement(elCarbon, 1);
  cf4Gas->AddElement(elFluorine, 4);
  
  // do dispersion function and fill arrays in loop
  G4double photonE[2] = {1239.841939*eV/140, 1239.841939*eV/1200};
  G4double cf4n[2] = {1.0, 1.0};
  // add scintillation here
  
  G4MaterialPropertiesTable *mpt_gascf4 = new G4MaterialPropertiesTable();
  mpt_gascf4->AddProperty("RINDEX", photonE, cf4n, 2); //2 is for length of arrays
  cf4Gas->SetMaterialPropertiesTable(mpt_gascf4);
  
  // ---------- Silicon PM ----------//
  G4Material * sipmSilicon = nist->FindOrBuildMaterial("G4_Si");
  
  // ---------- Alpha Source ----------//
  G4double goldFrac = 0.5;
  
  G4Element * elAm = new G4Element("Americium", "Am", 95, 243 * g/mole);
  G4Element * elO = new G4Element("Oxygen", "O", 8,  16. * g/mole);
  G4Material * mAmOxide = new G4Material("mAmOxide", 11.68 * g/cm3, 2);
  mAmOxide->AddElement(elAm, 1);
  mAmOxide->AddElement(elO, 2);
  
  G4Material * mGold = nist->FindOrBuildMaterial("G4_Au");
  
  G4double sourceDensity = mGold->GetDensity() * goldFrac + mAmOxide->GetDensity()*(1-goldFrac);
  G4Material * mSource = new G4Material("mSource", sourceDensity, 2);
  mSource->AddMaterial(mGold, goldFrac);
  mSource->AddMaterial(mAmOxide, 1-goldFrac);
  
  // ---------- Source casing ----------//
  G4Material * mSteel = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
  G4Material * mSilver = nist->FindOrBuildMaterial("G4_Ag");
  G4Material * mPalladium = nist->FindOrBuildMaterial("G4_Pd");
  
  ////////////////////------------ Building Detector ------------////////////////////
  // (lab space where gamma sources live)
  G4double world_sizeXY = 20*cm;
  G4double world_sizeZ  = 50*cm;

  G4Box* solidWorld = new G4Box("World", 0.5*world_sizeXY, 0.5*world_sizeXY, 0.5*world_sizeZ);
  
  G4LogicalVolume* logicWorld =                         
    new G4LogicalVolume(solidWorld,          //its solid
                        mWorld,           //its material
                        "World");            //its name
                                   
  G4VPhysicalVolume* physWorld = 
    new G4PVPlacement(0,                     //no rotation
                      G4ThreeVector(),       //at (0,0,0)
                      logicWorld,            //its logical volume
                      "World",               //its name
                      0,                     //its mother  volume
                      false,                 //no boolean operation
                      0,                     //copy number
                      checkOverlaps);        //overlaps checking
  
  /////////////////// Outer Cylinder ///////////////////
  // outer cylinder (vacuum?)
  // G4double outerCyl_r = 15*cm;
  // G4double outerCyl_thick = 1*cm;
  // G4double outerCyl_z = 100*cm;
  // G4Material* cylSteel = G4Material::GetMaterial("G4_STAINLESS-STEEL");
  // 
  // G4Tubs * solidOuterCyl = new G4Tubs("outerCyl", outerCyl_r, outerCyl_r + outerCyl_thick, 0.5*outerCyl_z, 0, 360.*deg);
  // G4LogicalVolume * logicOuterWorld = new G4LogicalVolume(solidOuterCyl, mWorld)       
  

  /////////////////// CF4 volume (Cylinder) ///////////////////
  // centered on XY place, goes from 0 to innerCyl_z on z axis
  // volume of CF4 held in cylinder
  G4double innerCyl_r = 4*cm;
  // G4double innerCyl_t = 1*cm; //thickness
  G4double innerCyl_z = 15*cm;
  
  G4Tubs * solid_cf4 = new G4Tubs("solid_cf4", 0., innerCyl_r, 0.5*innerCyl_z, 0, 360.*deg);
  
  G4LogicalVolume * logic_cf4 = new G4LogicalVolume(solid_cf4, cf4Gas, "logic_cf4");
  
  G4VPhysicalVolume* phys_cf4 = new G4PVPlacement(0,                    //no rotation
                    G4ThreeVector(0, 0, 0),      //at (0,0,0)
                    logic_cf4,            //its logical volume
                    "phys_cf4",           //its name
                    logicWorld,           //its mother  volume
                    false,                //no boolean operation
                    0,                    //copy number
                    checkOverlaps);       //overlaps checking
  
                    
  /////////////////// SiPM volume (Box) ///////////////////
  G4double sipmXY = 15*mm;
  G4double sipmThick = 5*mm;
  // G4double sipmPosition = -innerCyl_z/2 + 4*cm;
  G4double sipmZ = -sipmThick/2; 
  
  G4Box * solid_sipm = new G4Box("solid_sipm", 0.5*sipmXY, 0.5*sipmXY, 0.5*sipmThick);
    
  // sipmSilicon->GetMaterialTable();
  
  G4LogicalVolume * logic_sipm = new G4LogicalVolume(solid_sipm, sipmSilicon, "logic_sipm");
  G4VPhysicalVolume* phys_sipm = new G4PVPlacement(0,
                    G4ThreeVector(0, 0, sipmZ), // SiPM face is at (0, 0, 0)
                    logic_sipm,
                    "phys_sipm",
                    logic_cf4,         // mother volume
                    false, 
                    0,
                    checkOverlaps);
  //
  fScoringVolume = logic_cf4;
  fsipmVolume = logic_sipm;
  
  /////////////////////// Source ///////////////////////
  G4double sourceCaseRadius = 4.3/2 * mm; //zhiheng measured 4.2mm diameter
  G4double sourceRadius = 2.45/2 * mm; // from Amersham certificate of approval

  G4double palladiumThick = 1.8 * um;
  G4double goldThick = 0.2*um;
  G4double amo2Thick = 0.5 * um;
  G4double totalSourceThick = 0.2 * mm;
  G4double agThick = totalSourceThick - palladiumThick - goldThick - amo2Thick;
  
  G4double caseAboveSource = 0.65 * mm; // this much gap above source and top of case (sets solid angle)
  G4double caseSideHeight = 2.6 * mm;
  G4double caseBottomRadius = 6.0 * mm;
  G4double caseBottomThick = 0.5 * mm;
  G4double extraSSunderAg = caseSideHeight - caseAboveSource - totalSourceThick; // how much SS under Ag to make it full of metal
  
  // G4double sipmPosition = -innerCyl_z/2 + 4*cm;
  G4ThreeVector palCenter(0, 0, sourceToSipm + palladiumThick/2);
  G4ThreeVector goldCenter(0, 0, palCenter.getZ() + palladiumThick/2 + goldThick/2);
  G4ThreeVector amo2Center(0, 0, goldCenter.getZ() + goldThick/2 + amo2Thick/2);
  G4ThreeVector agCenter(0, 0, amo2Center.getZ() + amo2Thick/2 + agThick/2);
  G4ThreeVector ssUnderCenter(0, 0, agCenter.getZ() + agThick/2 + extraSSunderAg/2);
  
  G4ThreeVector caseSideCenter(0, 0, palCenter.getZ() + caseSideHeight/2 - caseAboveSource);
  
  G4Tubs * solid_palladium = new G4Tubs("solid_palladium", 0., sourceRadius, 0.5*palladiumThick, 0, 360.*deg);
  G4LogicalVolume * logic_palladium = new G4LogicalVolume(solid_palladium, mPalladium, "logic_palladium");
  G4VPhysicalVolume* phys_palladium = new G4PVPlacement(0,
                    palCenter,
                    logic_palladium,
                    "phys_palladium",
                    logic_cf4,         // mother volume
                    false, 
                    0,
                    checkOverlaps);
                    
  G4Tubs * solid_gold = new G4Tubs("solid_gold", 0., sourceRadius, 0.5*goldThick, 0, 360.*deg);
  G4LogicalVolume * logic_gold = new G4LogicalVolume(solid_gold, mGold, "logic_gold");
  G4VPhysicalVolume* phys_gold = new G4PVPlacement(0,
                    goldCenter,
                    logic_gold,
                    "phys_gold",
                    logic_cf4,         // mother volume
                    false, 
                    0,
                    checkOverlaps);

G4Tubs * solid_amo2 = new G4Tubs("solid_amo2", 0., sourceRadius, 0.5*amo2Thick, 0, 360.*deg);
G4LogicalVolume * logic_amo2 = new G4LogicalVolume(solid_amo2, mSource, "logic_amo2");
G4VPhysicalVolume* phys_amo2 = new G4PVPlacement(0,
                  amo2Center,
                  logic_amo2,
                  "phys_amo2",
                  logic_cf4,         // mother volume
                  false, 
                  0,
                  checkOverlaps);
                  
  G4PhysicalVolumeStore::GetInstance()->Register(phys_amo2);
                  
  G4Tubs * solid_Ag = new G4Tubs("solid_Ag", 0., sourceRadius, 0.5*agThick, 0, 360.*deg);
  G4LogicalVolume * logic_Ag = new G4LogicalVolume(solid_Ag, mSilver, "logic_Ag");
  G4VPhysicalVolume* phys_Ag = new G4PVPlacement(0,
                    agCenter,
                    logic_Ag,
                    "phys_Ag",
                    logic_cf4,         // mother volume
                    false, 
                    0,
                    checkOverlaps);
                    
  G4Tubs * solid_ssUnder = new G4Tubs("solid_caseSide", 0, sourceRadius, 0.5*extraSSunderAg, 0, 360.*deg);
  G4LogicalVolume * logic_ssUnder = new G4LogicalVolume(solid_ssUnder, mSteel, "logic_ssUnder");
  G4VPhysicalVolume* phys_ssUnder = new G4PVPlacement(0,
                ssUnderCenter,
                logic_ssUnder,
                "phys_caseSide",
                logic_cf4,         // mother volume
                false, 
                0,
                checkOverlaps);
                    
  G4Tubs * solid_caseSide = new G4Tubs("solid_caseSide", sourceRadius, sourceCaseRadius, 0.5*caseSideHeight, 0, 360.*deg);
  G4LogicalVolume * logic_caseSide = new G4LogicalVolume(solid_caseSide, mSteel, "logic_caseSide");
  G4VPhysicalVolume* phys_caseSide = new G4PVPlacement(0,
                  caseSideCenter,
                  logic_caseSide,
                  "phys_caseSide",
                  logic_cf4,         // mother volume
                  false, 
                  0,
                  checkOverlaps);
  

  
  // setting all headers
  fScoringVolume = logic_cf4;
  fsipmVolume = logic_sipm;

  //
  //always return the physical World
  //
  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
