#ifndef ALPHASOURCE1_HH
#define ALPHASOURCE1_HH 1

#include "G4LogicalVolume.hh"

class G4LogicalVolume;
class DetectorMaterials;

class alphaSource1 : public G4LogicalVolume{
  
public:
  alphaSource1(G4bool checkOverlaps);
  
  G4VPhysicalVolume * doPlacement(G4double zheight, G4LogicalVolume * mother, G4bool checkOverlaps);
  
  G4double GetSourceHeight();
  void SetSourceHeight();
  
private:
  
  G4double sourceHeight;
  DetectorMaterials * detMat;
  
  static G4double fMaxZ; // total source size (along axis towards SiPM)
  static G4double fMaxR;
};

#endif