#include "collimator.hh"

#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4Types.hh"

#include "G4Box.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"

#include "DetectorMaterials.hh"
#include "MaterialsMessenger.hh"

// local world volume of collimator
G4double collimator::fMaxZ = 10.0 * cm;
G4double collimator::fMaxX = 6. * cm; // 5 is required
G4double collimator::fMaxY = 6. * cm; // 4.5 is required, 6 is extra

collimator::collimator(G4bool checkOverlaps)
  : G4LogicalVolume(new G4Box("solid_collWorld", fMaxX/2, fMaxY/2, fMaxZ/2),
  DetectorMaterials::GetInstance()->mAir, "logic_collWorld"){
  
  dmat = DetectorMaterials::GetInstance();
  
  G4double lt = 2.25 * cm; // standard block size (LeadThickness)
  G4double leadLength = 10.*cm;
  G4double holeD = 0.4 * cm; // center hole diameter
  G4double crack = 0.05*mm; // paper can fit between lead blocks
  G4double negX_vgap = lt/2 + holeD/2;
  G4double posX_vgap = lt/2 + holeD/4;  
  
  // lead box
  G4Box * box_lead = new G4Box("solid_box1", lt/2, lt/2, leadLength/4);
  G4LogicalVolume * logic_lead = new G4LogicalVolume(box_lead, dmat->mLead, "logic_box1");
  
  // top two, then bottom two
  // ones on negative x have bigger vertical gap than positive x
  new G4PVPlacement(0, G4ThreeVector(lt/2 + crack, posX_vgap , 0), logic_lead, "phys_box1", this, 0, 0, checkOverlaps);
  new G4PVPlacement(0, G4ThreeVector(-lt/2, negX_vgap, 0), logic_lead, "phys_box1", this, 0, 0, checkOverlaps);
  new G4PVPlacement(0, G4ThreeVector(-lt/2, -negX_vgap, 0), logic_lead, "phys_box1", this, 0, 0, checkOverlaps);
  new G4PVPlacement(0, G4ThreeVector(lt/2 + crack, -posX_vgap, 0), logic_lead, "phys_box1", this, 0, 0, checkOverlaps);
  
}

G4VPhysicalVolume * collimator::doPlacement(G4double zheight, G4LogicalVolume * mother, G4bool checkOverlaps){
  
  G4ThreeVector lead_center(25*cm, 0*cm, 3*cm);

  G4RotationMatrix * rot_lead1 = new G4RotationMatrix();
  rot_lead1->rotateY(90*deg);
  rot_lead1->rotateZ(90*deg);
  
  return new G4PVPlacement(rot_lead1, //no rotation
                              lead_center,    // translation
                              this,    // material
                              "phys_collimator",   // name
                              mother,   // mother volume
                              false, 0, checkOverlaps); // boolean, copy number, checkOverlaps
}