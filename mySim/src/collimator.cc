#include "collimator.hh"

#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4Types.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4PhysicalVolumeStore.hh"

#include "DetectorMaterials.hh"

// enclosing volume of entire collimator object
G4double collimator::fMaxZ = 11.0 * cm;
G4double collimator::fMaxX = 6. * cm; // 5 is required
G4double collimator::fMaxY = 6. * cm; // 4.5 is required, 6 is extra

// -- physical placement -- //
G4VPhysicalVolume * collimator::doPlacement(G4double zheight, G4LogicalVolume * mother, G4bool checkOverlaps){
  
  // offset of cans + outer radius + outer wall thickness + extraSpace of collimator world + lead half length
  G4ThreeVector lead_center(-5.38*cm + 24.79*cm/2 + 0.61/2 * cm + fextraSpace + 5.*cm, 0*cm, zheight);

  G4RotationMatrix * rot_lead1 = new G4RotationMatrix();
  rot_lead1->rotateY(90*deg);

  return new G4PVPlacement(rot_lead1, //no rotation
                              lead_center,    // translation
                              this,    // material
                              "phys_collimator",   // name
                              mother,   // mother volume
                              false, 0, checkOverlaps); // boolean, copy number, checkOverlaps
}

// -- constructor -- //
collimator::collimator(G4bool checkOverlaps)
  : G4LogicalVolume(new G4Box("solid_collWorld", fMaxX/2, fMaxY/2, fMaxZ/2),
  DetectorMaterials::GetInstance()->mAir, "logic_collWorld"){
  
  dmat = DetectorMaterials::GetInstance();
  
  //-- build Lead Blocks
  // G4double lt = 2.25 * cm; // standard block size (LeadThickness)
  G4double leadLength = 10.*cm;
  fextraSpace = (collimator::fMaxZ - leadLength)/2;
  G4double extraSpace = fextraSpace;
  G4double holeD = 4. * mm; // center hole diameter
  G4double paper = 0.05*mm/2; // paper can fit between lead blocks
  G4double crack = 1.*mm;

  // block dimensions
  G4double CF4_w = 2.5*cm;
  G4double CF4_h = 2.5*cm;
  G4double b2_w = 2.*cm;
  G4double b2_h = 2.5*cm;
  G4double b3_w = 2.5*cm;
  G4double b3_h = 2.*cm;
  G4double b4_w = 2.*cm;
  G4double b4_h = 2.5*cm - 1.5*mm; // 1.5mm is extra
  
  // blocks 2, 4 are equal
  G4Box * solid_CF4 = new G4Box("solid_CF4", CF4_w/2, CF4_h/2, leadLength/2);
  G4Box * solid_b2 = new G4Box("solid_b2", b2_h/2, b2_w/2, leadLength/2);
  G4Box * solid_b3 = new G4Box("solid_b3", b3_h/2, b3_w/2, leadLength/2);
  G4Box * solid_b4 = new G4Box("solid_b4", b4_h/2, b4_w/2, leadLength/2);
  
  G4LogicalVolume * logic_CF4 = new G4LogicalVolume(solid_CF4, dmat->mLead, "logic_CF4");
  G4LogicalVolume * logic_b2 = new G4LogicalVolume(solid_b2, dmat->mLead, "logic_b2");
  G4LogicalVolume * logic_b3 = new G4LogicalVolume(solid_b3, dmat->mLead, "logic_b3");
  G4LogicalVolume * logic_b4 = new G4LogicalVolume(solid_b4, dmat->mLead, "logic_b4");
  
  G4double shift_x1 = CF4_h/2 + holeD/2;
  G4double shift_y1 = CF4_w/2 - holeD/2 + paper;
  
  G4double shift_x2 = b2_h/2 - holeD/2 + 2*paper;
  G4double shift_y2 = b2_w/2 + holeD/2;
  
  G4double shift_x3 = b3_h/2 + holeD/2;
  G4double shift_y3 = b3_w/2 - holeD/2;
  
  G4double shift_x4 = b4_h/2 - holeD/2 + crack;
  G4double shift_y4 = b4_w/2 + holeD/2;
  
  //big one, CF4, is on top right (+, +) quadrant
  new G4PVPlacement(0, G4ThreeVector(shift_x1, shift_y1 , extraSpace), logic_CF4, "phys_box1", this, 0, 0, checkOverlaps);
  new G4PVPlacement(0, G4ThreeVector(shift_x2, -shift_y2, extraSpace), logic_b2, "phys_box2", this, 0, 0, checkOverlaps);
  new G4PVPlacement(0, G4ThreeVector(-shift_x3, -shift_y3, extraSpace), logic_b3, "phys_box3", this, 0, 0, checkOverlaps);
  new G4PVPlacement(0, G4ThreeVector(-shift_x4, shift_y4, extraSpace), logic_b4, "phys_box4", this, 0, 0, checkOverlaps);
   
  // add plastic tube
  G4double plasticThick = 0.2*mm;
  G4Tubs * solid_plasticTube = new G4Tubs("solid_plasticTube", holeD/2 - plasticThick, holeD/2, leadLength/2, 0., 360.*deg);
  G4LogicalVolume * logic_plasticTube = new G4LogicalVolume(solid_plasticTube, dmat->mGenericPlastic, "logic_plasticTube");
  new G4PVPlacement(0, G4ThreeVector(0, 0, extraSpace), logic_plasticTube, "phys_plasticTube", this, 0, 0, checkOverlaps);
  
  //-- actual source 'holder' (this is a dummy item for Cs or Co sources)
  G4double gsR = 0.4*mm;
  G4double gsZ = 1.*mm;
  
  G4Tubs * solid_gammaSource = new G4Tubs("solid_gammaSource", 0, gsR, gsZ/2, 0, 360.*deg);
  G4LogicalVolume * logic_gammaSource = new G4LogicalVolume(solid_gammaSource, dmat->mGenericPlastic, "logic_gammaSource");
  G4VPhysicalVolume * phys_gs = new G4PVPlacement(0, G4ThreeVector(0, 0, -leadLength/2 -gsZ/2 + extraSpace), logic_gammaSource, "phys_gammaSource", this, 0, 0, checkOverlaps);
  //G4PhysicalVolumeStore::GetInstance()->Register(phys_gs); // to get volume elsewhere. Not needed. already registered with previous command

}