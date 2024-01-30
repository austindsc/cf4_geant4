#ifndef LongPassFilter_h
#define LongPassFilter_h 1

#include "globals.hh"
#include "G4VModularPhysicsList.hh"
#include "G4LogicalVolume.hh"

// forward declarations
class DetectorMaterials;

class LongPassFilter : public G4LogicalVolume{
  
public:
  LongPassFilter(G4int opticsStrategy = 0, G4bool checkOverlaps = false);
  ~LongPassFilter(){}
  
  // G4VPhysicalVolume * DoPlacement(G4ThreeVector * vec, G4RotationMatrix * rot, G4LogicalVolume * mother, G4bool checkOverlaps);
  // G4VPhysicalVolume * DoPlacement(G4VPhysicalVolume * theSiPMPackage, G4LogicalVolume * mother, G4bool checkOverlaps){}
  
  G4bool SetupOptics(G4double fixedTransmission = 0.9, G4String someOption = "default");

  // G4double 
  static const G4double mRadius;
  static const G4double mThick;
  static const G4double mSurfaceThick;
  static const G4double mEdgeBuffer;
  static const G4double mTotalThickness;

private:
  // don't need copy constructor or assignment (for now). could be useful with VUV4 though
  // LongPassFilter(const LongPassFilter &);
  // LongPassFilter & operator=(const LongPassFilter &);  DetectorMaterials * dmat;
  
  G4double mTransmission;

  G4VPhysicalVolume * mPhysBottom;
  G4VPhysicalVolume * mPhysSurface;
  G4VPhysicalVolume * mPhysTop;
  G4LogicalVolume * mLogBottom;
  G4LogicalVolume * mLogSurface;
};

#endif