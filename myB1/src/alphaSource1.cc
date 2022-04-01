#include "alphaSource1.hh"

#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4Types.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Polyhedra.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4PhysicalVolumeStore.hh"

#include "DetectorMaterials.hh"

// enclosing volume of entire collimator object
// G4double alphaSource1::fMaxZ = 2 * cm;
// G4double alphaSource1::fMaxR = 23. * mm; // unsure what is required
// 
// G4VPhysicalVolume * alphaSource1::doPlacement(G4double zheight, G4LogicalVolume * mother, G4bool checkOverlaps){
//   G4ThreeVector palFace(0, 0, zheight);
//   return new G4PVPlacement(0,
//               palFace,
//               this,
//               "phys_alphaSource",
//               mother,
//               false, 0, checkOverlaps);
// }

// -- constructor -- //
alphaSource1::alphaSource1(G4LogicalVolume * medium, G4double zheight, G4bool checkOverlaps)
  // : G4LogicalVolume(new G4Tubs("solid_alphaWorld", 0, fMaxR, fMaxZ/2, 0., 360.*deg),
  // DetectorMaterials::GetInstance()->GetDetectorMedium(), "logic_alphaWorld")
  {
  logic_local_medium = medium;
  detMat = DetectorMaterials::GetInstance();
  
  G4double sourceCaseRadius = 4.3/2 * mm; //zhiheng measured 4.2mm diameter
  G4double sourceRadius = 2.45/2 * mm; // from Amersham certificate of approval

  G4double palladiumThick = 1.8 * um;
  G4double goldThick = 0.2*um;
  G4double amo2Thick = 0.5 * um;
  G4double totalSourceThick = 0.2 * mm;
  G4double agThick = totalSourceThick - palladiumThick - goldThick - amo2Thick;
  
  G4double caseAboveSource = 0.65 * mm; // this much gap above source and top of case (sets solid angle)
  G4double caseSideHeight = 2.6 * mm;
  G4double caseBottomRadius = 6.0/2 * mm;
  G4double caseBottomThick = 1. * mm;
  G4double extraSSunderAg = caseSideHeight - caseAboveSource - totalSourceThick; // how much SS under Ag to make it full of metal
  
  //screw and aluminum plate holding it
  G4double screwToAl_z = 9. * mm;
  G4double screwToAl_r = 2.4/2 * mm;
  // G4double alPlateSide = 23. * mm; // very rough guess
  G4double alPlateSide = 17. * mm; // very rough guess
  G4double alPlateThick = 4.*mm; // rougher guess
  
  // G4double sipmPosition = -innerCyl_z/2 + 4*cm;
  // G4double startOffset = -fMaxZ/2 + caseAboveSource;
  G4ThreeVector palCenter(0, 0, zheight + palladiumThick/2);
  G4ThreeVector goldCenter(0, 0, palCenter.getZ() + palladiumThick/2 + goldThick/2);
  G4ThreeVector amo2Center(0, 0, goldCenter.getZ() + goldThick/2 + amo2Thick/2);
  G4ThreeVector agCenter(0, 0, amo2Center.getZ() + amo2Thick/2 + agThick/2);
  G4ThreeVector ssUnderCenter(0, 0, agCenter.getZ() + agThick/2 + extraSSunderAg/2);
  
  G4ThreeVector caseSideCenter(0, 0, palCenter.getZ() + caseSideHeight/2 - caseAboveSource);
  G4ThreeVector caseBottomCenter(0, 0, caseSideCenter.getZ() + caseSideHeight/2 + caseBottomThick/2);
  
  G4ThreeVector screwToAlCenter(0, 0, caseBottomCenter.getZ() + caseBottomThick/2 + screwToAl_z/2);
  G4ThreeVector alPlateCenter(0, 0, screwToAlCenter.getZ() + screwToAl_z/2 +alPlateThick/2);
  
  G4Tubs * solid_palladium = new G4Tubs("solid_palladium", 0., sourceRadius, 0.5*palladiumThick, 0, 360.*deg);
  G4LogicalVolume * logic_palladium = new G4LogicalVolume(solid_palladium, detMat->mPalladium, "logic_palladium");
  G4VPhysicalVolume* phys_palladium = new G4PVPlacement(0,
                    palCenter,
                    logic_palladium,
                    "phys_palladium",
                    logic_local_medium,         // mother volume
                    false, 0, checkOverlaps);
                    
  G4Tubs * solid_gold = new G4Tubs("solid_gold", 0., sourceRadius, 0.5*goldThick, 0, 360.*deg);
  G4LogicalVolume * logic_gold = new G4LogicalVolume(solid_gold, detMat->mGold, "logic_gold");
  G4VPhysicalVolume* phys_gold = new G4PVPlacement(0,
                    goldCenter,
                    logic_gold,
                    "phys_gold",
                    logic_local_medium,         // mother volume
                    false, 0, checkOverlaps);

G4Tubs * solid_amo2 = new G4Tubs("solid_amo2", 0., sourceRadius, 0.5*amo2Thick, 0, 360.*deg);
G4LogicalVolume * logic_amo2 = new G4LogicalVolume(solid_amo2, detMat->mAmActiveMatrix, "logic_amo2");
G4VPhysicalVolume* phys_amo2 = new G4PVPlacement(0,
                  amo2Center,
                  logic_amo2,
                  "phys_amo2",
                  logic_local_medium,         // mother volume
                  false, 0, checkOverlaps);
                  
  G4PhysicalVolumeStore::GetInstance()->Register(phys_amo2); // to get volume elsewhere. Not needed?
                  
  G4Tubs * solid_Ag = new G4Tubs("solid_Ag", 0., sourceRadius, 0.5*agThick, 0, 360.*deg);
  G4LogicalVolume * logic_Ag = new G4LogicalVolume(solid_Ag, detMat->mSilver, "logic_Ag");
  G4VPhysicalVolume* phys_Ag = new G4PVPlacement(0,
                    agCenter,
                    logic_Ag,
                    "phys_Ag",
                    logic_local_medium,         // mother volume
                    false, 0, checkOverlaps);
                    
  G4Tubs * solid_ssUnder = new G4Tubs("solid_caseSide", 0, sourceRadius, 0.5*extraSSunderAg, 0, 360.*deg);
  G4LogicalVolume * logic_ssUnder = new G4LogicalVolume(solid_ssUnder, detMat->mSteel, "logic_ssUnder");
  G4VPhysicalVolume* phys_ssUnder = new G4PVPlacement(0,
                ssUnderCenter,
                logic_ssUnder,
                "phys_caseSide",
                logic_local_medium,         // mother volume
                false, 0, checkOverlaps);
                    
  G4Tubs * solid_caseSide = new G4Tubs("solid_caseSide", sourceRadius, sourceCaseRadius, 0.5*caseSideHeight, 0., 360.*deg);
  G4LogicalVolume * logic_caseSide = new G4LogicalVolume(solid_caseSide, detMat->mSteel, "logic_caseSide");
  G4VPhysicalVolume* phys_caseSide = new G4PVPlacement(0,
                  caseSideCenter,
                  logic_caseSide,
                  "phys_caseSide",
                  logic_local_medium,         // mother volume
                  false, 0, checkOverlaps);
                  
  G4Tubs * solid_caseBottom = new G4Tubs("solid_caseBottom", 0, caseBottomRadius, caseBottomThick/2, 0., 360.*deg);
  G4LogicalVolume * logic_caseBottom = new G4LogicalVolume(solid_caseBottom, detMat->mSteel, "logic_caseBottom");
  G4VPhysicalVolume* phys_caseBottom = new G4PVPlacement(0,
                  caseBottomCenter,
                  logic_caseBottom,
                  "phys_caseBottom",
                  logic_local_medium,         // mother volume
                  false, 0, checkOverlaps);
                  
  G4Tubs * solid_screwToAl = new G4Tubs("solid_screwToAl", 0, screwToAl_r, 0.5*screwToAl_z, 0., 360.*deg);
  G4LogicalVolume * logic_screwToAl = new G4LogicalVolume(solid_screwToAl, detMat->mSteel, "logic_screwToAl");
  G4VPhysicalVolume* phys_screwToAl = new G4PVPlacement(0,
                  screwToAlCenter,
                  logic_screwToAl,
                  "phys_screwToAl",
                  logic_local_medium,         // mother volume
                  false, 0, checkOverlaps);
  
  const G4double rInner[3] = {0*mm, 0*mm, 0*mm};                
  const G4double rOuter[3] = {alPlateSide/2*mm, alPlateSide/2*mm, alPlateSide/2*mm};
  const G4double zPlane[2] = {-alPlateThick/2, +alPlateThick/2};
  G4Polyhedra * solid_triPlate = new G4Polyhedra("solid_triPlate", 0., 360.*deg, 3, 2, zPlane, rInner, rOuter);
  
  G4LogicalVolume * logic_alPlate = new G4LogicalVolume(solid_triPlate, detMat->mAluminum, "logic_alPlate");
  G4VPhysicalVolume* phys_alPlate = new G4PVPlacement(0,
                  alPlateCenter,
                  logic_alPlate,
                  "phys_alPlate",
                  logic_local_medium,         // mother volume
                  false, 0, checkOverlaps);
                  

}