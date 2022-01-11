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
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();

  // Option to switch on/off checking of volumes overlaps
  G4bool checkOverlaps = true;

  /////////////////// World ///////////////////
  // (lab space where gamma sources live)
  G4double world_sizeXY = 20*cm;
  G4double world_sizeZ  = 50*cm;
  G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");

  G4Box* solidWorld = new G4Box("World", 0.5*world_sizeXY, 0.5*world_sizeXY, 0.5*world_sizeZ);
      
  G4LogicalVolume* logicWorld =                         
    new G4LogicalVolume(solidWorld,          //its solid
                        world_mat,           //its material
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
  // G4LogicalVolume * logicOuterWorld = new G4LogicalVolume(solidOuterCyl, world_mat)       
  
  /////////////////// CF4 volume (Cylinder) ///////////////////
  // centered on XY place, goes from 0 to innerCyl_z on z axis
  // volume of CF4 held in cylinder
  G4double innerCyl_r = 4*cm;
  // G4double innerCyl_t = 1*cm; //thickness
  G4double innerCyl_z = 15*cm;
  
  G4Element * elCarbon = new G4Element("Carbon", "C", 6, 12.01 * g/mole);
  G4Element * elFluorine = new G4Element("Fluorine", "F", 9,  19. * g/mole);
  
  G4double densityCF4 = 0.002*g/cm3; // slightly denser than STP air

  G4Material * cf4Gas = new G4Material("cf4Gas", densityCF4, 2);
  cf4Gas->AddElement(elCarbon, 1);
  cf4Gas->AddElement(elFluorine, 4);
  
  G4Tubs * solid_cf4 = new G4Tubs("solid_cf4", 0., innerCyl_r, 0.5*innerCyl_z, 0, 360.*deg);
  G4LogicalVolume * logic_cf4 = new G4LogicalVolume(solid_cf4, cf4Gas, "logic_cf4");
  new G4PVPlacement(
                    0,                    //no rotation
                    G4ThreeVector(0, 0, innerCyl_z/2),      //at (0,0,0)
                    logic_cf4,            //its logical volume
                    "phys_cf4",           //its name
                    logicWorld,           //its mother  volume
                    false,                //no boolean operation
                    0,                    //copy number
                    checkOverlaps);       //overlaps checking
  
                    
  /////////////////// SiPM volume (Box) ///////////////////
  G4double sipmXY = 15*mm;
  G4double sipmZ = 5*mm;
  G4double sipmHeight = 10*cm;
  
  G4Box * solid_sipm = new G4Box("solid_sipm", 0.5*sipmXY, 0.5*sipmXY, 0.5*sipmZ);
  
  G4Material * sipmSilicon = nist->FindOrBuildMaterial("G4_SI");
  
  G4LogicalVolume * logic_sipm = new G4LogicalVolume(solid_sipm, sipmSilicon, "logic_sipm");
  new G4PVPlacement(0,
                    G4ThreeVector(0, 0, sipmHeight),
                    logic_sipm,
                    "phys_sipm",
                    logicWorld,         // mother volume
                    false, 
                    0,
                    checkOverlaps);
  //
  fScoringVolume = logic_cf4;

  //
  //always return the physical World
  //
  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
