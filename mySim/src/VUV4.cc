#include "VUV4.hh"

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
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"

#include "G4LogicalSkinSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4OpticalSurface.hh"

#include "DetectorMaterials.hh"
#include "MyGlobals.hh"

// entire package info
const G4double VUV4::fPackageThick = 2.5*mm;
const G4double VUV4::fPackageXY = 15.*mm;
const G4double VUV4::fPackageZgap = 1.3*mm;
const G4double VUV4::fPackageSideGap = 1.5*mm;
const G4double VUV4::fPackageChannelGap = 7 * mm;
const G4double VUV4::fEdgeBuffer = 1*um;

// silicon quadrants info
const G4double VUV4::fSipmMiddleGap = 0.5*mm;
const G4double VUV4::fSipmXY = 5.9 *  mm; // in reality it is 5.95 x 5.85
const G4double VUV4::fSipmThick = 0.5*mm; // guess
const G4double VUV4::fSipmSurfaceThick = DETECTORSETUP::GetVisibleStatus() ? 0.2*mm : 110*nm; // 110nm if visible is false

const G4double VUV4::fWindowThick = 0.5*mm;
const G4double VUV4::fWindowFromSurface = 0.1*mm;

// extra medium on top
const G4double VUV4::fExtraMediumTopThick = DETECTORSETUP::GetVisibleStatus() ? 0.2*mm : 1*um; // 1um if visible is false

//------------ building sipm ------------//
// always gets built with of the detection medium

// need to build a second box of the medium, only because we need a physical volume to do the borders in the constructor.
// very annoying but the only way

VUV4::VUV4(G4double zheight, G4bool doOpticalSurfaces, G4bool buildWindow, G4bool checkOverlaps) :
G4LogicalVolume(new G4Box("logic_sipm_container", fPackageXY/2 + fEdgeBuffer, fPackageXY/2+fEdgeBuffer, fPackageThick/2+fEdgeBuffer), DetectorMaterials::GetInstance()->GetDetectorMedium(), "logic_sipmBox"){
  DetectorMaterials * detMat = DetectorMaterials::GetInstance();
  #include "colours.icc"

  // everything is done wrt this dummy box because we have to do this annoying optical borders (see bottom)
  // we need an emcompassing physical volume around the extraMedium for doing the optical borders

  // -- dummy big box -- //
  // Build and place another box of the exact same size as the base class, inside this
  G4Box * solid_innerBox = new G4Box("solid_innerSipmBox", 0.5*fPackageXY, 0.5*fPackageXY, 0.5*fPackageThick);
  G4LogicalVolume * logic_medium_thisBox = new G4LogicalVolume(solid_innerBox, detMat->GetDetectorMedium(), "logic_medium_thisBox");
  G4VPhysicalVolume* phys_medium_thisBox = new G4PVPlacement(0, G4ThreeVector(0, 0, 0),
                    logic_medium_thisBox,
                    "phys_innerBox",
                    (G4LogicalVolume *) this,
                    false, 0, checkOverlaps);
  logic_medium_thisBox->SetVisAttributes(world_transparent);

  //-- ceramic package --//
  G4Box * solid_BoxPackage = new G4Box("solid_BoxPackage", 0.5*fPackageXY, 0.5*fPackageXY, 0.5*fPackageThick); // full box
  G4Box * solid_allQuads_toSubtract = new G4Box("solid_allQuads", 0.5*(2*fSipmXY + fSipmMiddleGap + fPackageSideGap), 0.5*(2*fSipmXY + fSipmMiddleGap + fPackageSideGap), 0.5*(fPackageZgap + fSipmThick)); // center square to subtract
  G4Box * solid_sideChannelToSubtract = new G4Box("solid_RectangleSubtract", 0.5*(1.5*fPackageXY), 0.5*fPackageChannelGap, solid_allQuads_toSubtract->GetZHalfLength()); // this is to add the channel (gap) in the side of packaging
  
  G4SubtractionSolid * sub_package = new G4SubtractionSolid("fullBorder_solid_package", solid_BoxPackage, solid_allQuads_toSubtract,
              0, G4ThreeVector(0, 0, solid_BoxPackage->GetZHalfLength() - solid_allQuads_toSubtract->GetZHalfLength())); // this is right
  sub_package = new G4SubtractionSolid("solid_package", sub_package, solid_sideChannelToSubtract, 
              0, G4ThreeVector(0, 0, + solid_BoxPackage->GetZHalfLength() - solid_allQuads_toSubtract->GetZHalfLength() + 0.5*fSipmThick)); // not sure if + sipmThick is right
  G4LogicalVolume * logic_package = new G4LogicalVolume(sub_package, detMat->mCeramic, "logic_package");
  logic_package->SetVisAttributes(ceramic_White);
  
  // top of package is at origin!!! (do that in detector construction)
  G4ThreeVector movePackage = G4ThreeVector(0, 0, 0);
  G4VPhysicalVolume* phys_package = new G4PVPlacement(0, movePackage, logic_package, "phys_package", logic_medium_thisBox, false, 0, checkOverlaps); // BEGIN: ed8c6549bwf9

  // move package puts sipm at center of package  (again, top of package is at origin)
  // logic of moving SiPM (written out bc I keep getting confused)
  // 1. down by half the package (sipm middle at bottom of package)
  // 2. up by half sipm thickness (sipm bottom at bottom of package)
  // 3. up by the difference between boxPackage and subtracted package
  G4double goodZ = 0;
  goodZ = -solid_BoxPackage->GetZHalfLength();
  goodZ += fSipmThick/2.;
  goodZ += (fPackageThick - 2*solid_allQuads_toSubtract->GetZHalfLength());

  // // make the Silicon squares
  G4double local_sipmFullxy = 2*fSipmXY + fSipmMiddleGap;
  G4double xy, z, middleGap;
  G4String solidName;
  G4SubtractionSolid* solid_siBoxes = VUV4::Create4Grid(xy = local_sipmFullxy, z = fSipmThick, middleGap = fSipmMiddleGap, solidName = "solid_siBoxes");

  G4LogicalVolume * logic_si_boxes = new G4LogicalVolume(solid_siBoxes, detMat->mSilicon, "logic_si_boxes");
  fSensitiveSilicon = logic_si_boxes; // VERY IMPORTANT
  logic_si_boxes->SetVisAttributes(si_magenta);

  G4ThreeVector sipmPos_rel_ceramic = movePackage + G4ThreeVector(0, 0, goodZ);
  G4VPhysicalVolume* phys_si_boxes = new G4PVPlacement(0, sipmPos_rel_ceramic, logic_si_boxes, "phys_si_boxes", logic_medium_thisBox, false, 0, checkOverlaps);

  // make another cross out of SiO2
  G4SubtractionSolid * solid_passivationBoxes = Create4Grid(xy = local_sipmFullxy, z = fSipmSurfaceThick, middleGap = fSipmMiddleGap, solidName = "solid_passivationBoxes");
  G4LogicalVolume * logic_passivationBoxes = new G4LogicalVolume(solid_passivationBoxes, detMat->mQuartz, "logic_passivationBoxes");
  logic_passivationBoxes->SetVisAttributes(lblue);
  G4double surfaceMoveZ = goodZ + fSipmThick/2. + fSipmSurfaceThick/2.;
  G4ThreeVector moveSipmSurface = movePackage + G4ThreeVector(0, 0, surfaceMoveZ);
  
  if(DETECTORSETUP::GetSurfaceStatus() == true){
    G4VPhysicalVolume* phys_passivationBoxes = new G4PVPlacement(0, moveSipmSurface, logic_passivationBoxes, "phys_passivationBoxes", logic_medium_thisBox, false, 0, checkOverlaps);
  }

  //---- adding extra medium above boxes ----//
  // fExtraMediumTopThick = declared at top of file as const // very thin box for doing extra optical layers in medium
  G4double extraBox_xy = 2*solid_allQuads_toSubtract->GetXHalfLength(); // fill the entire top of SiPM with this (thin) surface
  G4Box* solid_additionalBox = new G4Box("solid_additionalBox", extraBox_xy/2, extraBox_xy/2, fExtraMediumTopThick/2);
  G4LogicalVolume* logic_additionalBox = new G4LogicalVolume(solid_additionalBox, detMat->GetDetectorMedium(), "logic_additionalBox");
  logic_additionalBox->SetVisAttributes(world_transparent);
  if(DETECTORSETUP::GetVisibleStatus() == true) logic_additionalBox->SetVisAttributes(filter_yellow);
  
  G4double extraBoxMoveZ = surfaceMoveZ + fSipmSurfaceThick/2. + fExtraMediumTopThick/2.;
  G4ThreeVector moveAdditionalBox = movePackage + G4ThreeVector(0, 0, extraBoxMoveZ);
  G4VPhysicalVolume * physExtraMedium = new G4PVPlacement(0, moveAdditionalBox, logic_additionalBox, "phys_additionalBox", logic_medium_thisBox, false, 0, checkOverlaps);


  if(doOpticalSurfaces){
    // adding diffusion to ceramic
    G4LogicalSkinSurface * ls_package = new G4LogicalSkinSurface("opSkin_ceramic", logic_package, (G4SurfaceProperty*)detMat->opSurf_ceramic);
    
    // dielectric_metal of Si bulk
    new G4LogicalSkinSurface("opSurf_siBulk", logic_si_boxes, (G4SurfaceProperty*) detMat->opSurf_silicon); // silicon bulk, use metal dielectric with Transmission = 100
    
    // can't build these without the surface, otherwise photons get absorbed between two of the same medium
    // as there is no SiO2 layer to create the correct boundary ordering
    if(DETECTORSETUP::GetSurfaceStatus() == true){
      // doing my PDE correction (diffusion in)
      new G4LogicalBorderSurface("opSurf_myCorrector1_in", phys_medium_thisBox, physExtraMedium, (G4SurfaceProperty*) detMat->opSurf_myCorrector1_diffuserIn);

      // my PDE correction (absorption on way out)
      new G4LogicalBorderSurface("opSurf_myCorrector2_out", physExtraMedium, phys_medium_thisBox, (G4SurfaceProperty*) detMat->opSurf_myCorrector2_absorptionOut);
    }
  }

  if(buildWindow){
    // build window flush with the top of the ceramic package
    G4Box* solid_window = new G4Box("solid_window", solid_allQuads_toSubtract->GetXHalfLength(), solid_allQuads_toSubtract->GetYHalfLength(), fWindowThick/2);
    G4LogicalVolume* logic_window = new G4LogicalVolume(solid_window, detMat->mQuartz, "logic_window");
    logic_window->SetVisAttributes(window_gray);

    // calculate the position of the window
    G4ThreeVector moveWindow = movePackage + G4ThreeVector(0, 0, solid_BoxPackage->GetZHalfLength() - fWindowThick/2 - fWindowFromSurface);

    // place the window
    G4VPhysicalVolume* phys_window = new G4PVPlacement(0, moveWindow, logic_window, "phys_window", logic_medium_thisBox, false, 0, checkOverlaps);

  }

  return;
}

// extra function to make a quadrant of objects (sipms and their surfaces)
// can call from anywhere with VUV4::Create4Grid(...) because its static in the header
G4SubtractionSolid* VUV4::Create4Grid(G4double xy, G4double z, G4double middleGap, G4String solid_name){
  G4double overdoEdges = 1.1;
  G4Box* si_totalBox = new G4Box("solid_totalBox", 0.5*xy, 0.5*xy, 0.5*z);
  G4Box* solid_cross1 = new G4Box("solid_cross1", overdoEdges * 0.5*xy, 0.5*middleGap, overdoEdges*z);
  G4Box* solid_cross2 = new G4Box("solid_cross2", 0.5*middleGap, overdoEdges*0.5*xy, overdoEdges*z);
  G4UnionSolid* solidCross = new G4UnionSolid("solidCross", solid_cross1, solid_cross2);
  G4SubtractionSolid* sub_totalBox = new G4SubtractionSolid(solid_name, si_totalBox, solidCross);
  return sub_totalBox;
}