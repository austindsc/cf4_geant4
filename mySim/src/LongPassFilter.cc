#include "LongPassFilter.hh"

#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4Types.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4PhysicalVolumeStore.hh"

// optics
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4MaterialPropertiesTable.hh"

// my includes
#include "DetectorMaterials.hh"
#include "HelperFunctions.hh"
#include "MyGlobals.hh"

const G4double LongPassFilter::mRadius = 25.4/2 * mm;
const G4double LongPassFilter::mThick = 1.1 * mm;
const G4double LongPassFilter::mSurfaceThick = DETECTORSETUP::GetVisibleStatus() ? 1*mm : 0.01*mm;
const G4double LongPassFilter::mEdgeBuffer = 0.001*mm;
const G4double LongPassFilter::mTotalThickness = mThick + mSurfaceThick + mEdgeBuffer;
// from https://www.newport.com/p/10CGA-495

LongPassFilter::LongPassFilter(G4int opticsStrategy, G4bool checkOverlaps) : G4LogicalVolume(
  new G4Tubs("solid_LPFilter", 0.0, mRadius+mEdgeBuffer, mTotalThickness/2, 0.0, 360.0 * deg), // make disk
  DetectorMaterials::GetInstance()->GetDetectorMedium(), "logic_LPFilter") // done inherited constructor
  {
    DetectorMaterials * detMat = DetectorMaterials::GetInstance();
    #include "colours.icc"

    this->SetVisAttributes(G4VisAttributes::GetInvisible()); // want the 'world medium' to be transparent
    
    // filter body
    G4Tubs * filter_disk = new G4Tubs("solid_LPFilter", 0.0, mRadius, mThick/2, 0.0, 360.0 * deg);
    mLogBottom = new G4LogicalVolume(filter_disk, detMat->mQuartz, "logic_LPFilter");
    mLogBottom->SetVisAttributes(filter_yellow);
    G4ThreeVector moveBottom = G4ThreeVector(0, 0, -mSurfaceThick/2);
    mPhysBottom = new G4PVPlacement(0, moveBottom, mLogBottom, "phys_lpFilter", this, false, 0, checkOverlaps);

    // filter surface/top
    G4Tubs * filterTop_disk = new G4Tubs("filterTop_disk", 0.0, mRadius, mSurfaceThick/2, 0.0, 360.0 * deg);
    mLogSurface = new G4LogicalVolume(filterTop_disk, detMat->mBoroSilicate, "logic_LPFilterTop");
    mLogSurface->SetVisAttributes(filter_green);
    G4ThreeVector moveSurface = moveBottom + G4ThreeVector(0, 0, mThick/2 + mSurfaceThick/2);
    mPhysSurface = new G4PVPlacement(0, moveSurface, mLogSurface, "phys_lpFilterTop", this, false, 0, checkOverlaps);

    this->SetupOptics();
  return;
}

G4bool LongPassFilter::SetupOptics(G4double fixedTransmission, G4String someOption){
    DetectorMaterials * detMat = DetectorMaterials::GetInstance();

    // filter curves for photons entering and exiting the surface
    new G4LogicalSkinSurface("filterSkin_surface", mLogSurface, detMat->opSurf_lpFilter); // doens't seem to work for exiting photons

    // ensure 100% transmission for photons between two layers of filter
    // logical border takes priority over logical skin
    new G4LogicalBorderSurface("filterInterface_in", mPhysSurface, mPhysBottom, detMat->opSurf_perfectTrans); // comment these out to do fresnel from body to filter top (n are mismatched) // case A, B
    new G4LogicalBorderSurface("filterInterface_out", mPhysBottom, mPhysSurface, detMat->opSurf_perfectTrans); // both of these mean perfect transmission in and out //case A, B

}