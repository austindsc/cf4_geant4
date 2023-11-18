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
#include "HelperFunctions.hh"
#include "MyGlobals.hh"

G4double LongPassFilter::mRadius = 25.4/2 * mm;
G4double LongPassFilter::mThick = 1.1 * mm;
G4double LongPassFilter::mSurfaceThick = 0.1 * mm;
// from https://www.newport.com/p/10CGA-495

LongPassFilter::LongPassFilter(G4bool checkOverlaps) : G4LogicalVolume(
  new G4Tubs("solid_LPFilter", 0.0, mRadius, mThick/2, 0.0, 360.0 * deg), // make disk
  DetectorMaterials::GetInstance()->mQuartz, "logic_LPFilter") // done inherited constructor
  {
  // // do all the optics you want to here
  // this->SetupOptics("data/lpfilter_500nm.txt", 0.69); // second value is not used rn. supposed to be for testing


}

bool LongPassFilter::SetupOptics(const std::string& fname, G4double Transmission){

  // if(MG::GetVerbosity() > 1){ // just for easy verbosity
  //   G4cout << "in longpassFilter.cc, these are the transmission values" << G4endl;
  //   for (size_t i = 0; i < wlsE_filter.size(); i++){
  //     G4double ref = 1.0 - transmission_filter[i];
  //     G4cout << 1240/(wlsE_filter[i]/eV) << "nm: ref = 1 - " << transmission_filter[i] << " = " << ref << G4endl;
  //   }
  // }
 
  // return true;
  // need to put all 
}

// bool LongPassFilter::DoPlacement(G4ThreeVector * vec, G4RotationMatrix * rot, G4LogicalVolume * mother, G4bool checkOverlaps){

// }