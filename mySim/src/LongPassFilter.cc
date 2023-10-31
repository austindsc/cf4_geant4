#include "LongPassFilter.hh"

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

// optics
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4MaterialPropertiesTable.hh"

// my includes
#include "DetectorMaterials.hh"

G4double LongPassFilter::mRadius = 25.4/2 * mm;
G4double LongPassFilter::mThick = 1.1 * mm;
// from https://www.newport.com/p/10CGA-495

LongPassFilter::LongPassFilter(G4bool checkOverlaps) : G4LogicalVolume(
  new G4Tubs("solid_LPFilter", 0.0, mRadius, mThick/2, 0.0, 360.0 * deg), // make disk
  DetectorMaterials::GetInstance()->GetDetectorMedium(), "logic_LPFilter"){

  // do all the optics you want to here
  this->SetupOptics("blah.txt", 0.69);

}

bool LongPassFilter::SetupOptics(const std::string& fname, G4double Transmission){


  return true;
}