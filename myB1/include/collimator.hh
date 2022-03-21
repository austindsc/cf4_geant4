// lets try putting multiple objects into one

#ifndef COLLIMATOR_H
#define COLLIMATOR_H 1

#include "globals.hh"
#include "G4LogicalVolume.hh"

class G4LogicalVolume;
class DetectorMaterials;

class collimator : public G4LogicalVolume{
  
public:
  collimator(G4bool checkOverlaps);
  
  G4VPhysicalVolume * doPlacement(G4double zheight, G4LogicalVolume * mother, G4bool checkOverlaps);
  
  G4double GetColTest();
  
private:
  G4LogicalVolume* logic_coll;
  G4double gammaHeight;
  DetectorMaterials * dmat;
  
  static G4double fMaxZ; // along direction gammas shoot (different than CF4 z axis)
  static G4double fMaxX;
  static G4double fMaxY;
};

#endif