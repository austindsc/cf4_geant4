#ifndef LongPassFilter_h
#define LongPassFilter_h 1

#include "globals.hh"
#include "G4VModularPhysicsList.hh"
#include "G4LogicalVolume.hh"

// forward declarations
class DetectorMaterials;

class LongPassFilter : public G4LogicalVolume{
  
public:
  LongPassFilter(G4bool checkOverlaps);
  
  G4VPhysicalVolume * doPlacement(G4ThreeVector * vec, G4RotationMatrix * rot, G4LogicalVolume * mother, G4bool checkOverlaps);
  
  bool SetupOptics(const std::string& fname, G4double Transmission);

private:
  DetectorMaterials * dmat;
  
  G4double mTransmission;

  static G4double mRadius;
  static G4double mThick;
};

#endif