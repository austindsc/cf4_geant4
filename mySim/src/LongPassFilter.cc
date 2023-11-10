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
// from https://www.newport.com/p/10CGA-495

LongPassFilter::LongPassFilter(G4bool checkOverlaps) : G4LogicalVolume(
  new G4Tubs("solid_LPFilter", 0.0, mRadius, mThick/2, 0.0, 360.0 * deg), // make disk
  DetectorMaterials::GetInstance()->mQuartz, "logic_LPFilter") // done inherited constructor
  {
  // do all the optics you want to here
  this->SetupOptics("data/lpfilter_500nm.txt", 0.69); // second value is not used rn. supposed to be for testing

}

bool LongPassFilter::SetupOptics(const std::string& fname, G4double Transmission){

  std::vector<G4double> wlsE_filter, transmission_filter, reflectivity_filter; // get out photon energy and transmission
  OH::ReadAndFillArray(fname, wlsE_filter, transmission_filter, MG::GetVerbosity(), 1, 1./100., true); // scale transmission values by 100, flip wavelengths to eV
  for (std::vector<G4double>::iterator it = transmission_filter.begin(); it != transmission_filter.end(); it++){
    G4double ref = 1.0 - *it;
    reflectivity_filter.push_back(ref);
  }

  if(MG::GetVerbosity() > 1){ // just for easy verbosity
    G4cout << "in longpassFilter.cc, these are the transmission values" << G4endl;
    for (size_t i = 0; i < wlsE_filter.size(); i++){
      G4double ref = 1.0 - transmission_filter[i];
      G4cout << 1240/(wlsE_filter[i]/eV) << "nm: ref = 1 - " << transmission_filter[i] << " = " << ref << G4endl;
    }
  }
  
  // actual values
  // G4MaterialPropertiesTable* filterOpticsTable = new G4MaterialPropertiesTable();
  // filterOpticsTable->AddProperty("REFLECTIVITY", wlsE_filter, reflectivity_filter);
  // filterOpticsTable->AddProperty("TRANSMITTANCE", wlsE_filter, transmission_filter);
  // filterOpticsTable->DumpTable();

  // -- making material
  // G4Material* customMaterial = new G4Material("CustomMaterial", density, elements, ncomponents);


  // test values - apply  to the physical objects
  // G4double trans = Transmission;
  // G4double ref = 1. - Transmission;

  // G4MaterialPropertiesTable* opticalSurfaceProperties = new G4MaterialPropertiesTable();
  // opticalSurfaceProperties->AddConstProperty("REFLECTIVITY", reflectivity);
  // opticalSurfaceProperties->AddConstProperty("TRANSMITTANCE", transmission);

  // G4OpticalSurface* myOpticalSurface = new G4OpticalSurface("MyOpticalSurface");
  // myOpticalSurface->SetType(dielectric_dielectric);
  // myOpticalSurface->SetModel(unified);
  // myOpticalSurface->SetMaterialPropertiesTable(opticalSurfaceProperties);

  // G4LogicalBorderSurface* surfaceBorder = new G4LogicalBorderSurface("SurfaceBorder", physicalVolume1, physicalVolume2, myOpticalSurface);


  // exit(0);
  return true;
}

// bool LongPassFilter::DoPlacement(G4ThreeVector * vec, G4RotationMatrix * rot, G4LogicalVolume * mother, G4bool checkOverlaps){

// }