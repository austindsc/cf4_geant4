#ifndef ALPHASOURCE1_HH
#define ALPHASOURCE1_HH 1

#include "G4LogicalVolume.hh"

class G4LogicalVolume;
class DetectorMaterials;

// previously used same framework as gammma source + collimator but
// didn't want to deal with multiple CF4 logical/physical volumes to
// track energy deposition. So this is essentially a wrapper for the alphaSource 
// source code.

class alphaSource1{
  
public:
  alphaSource1(G4LogicalVolume* medium, G4double zheight, G4bool checkOverlaps);
  
private:
  G4LogicalVolume * logic_local_medium;
  DetectorMaterials * detMat;

};

#endif