#ifndef B1DetectorConstructionMessenger_h
#define B1DetectorConstructionMessenger_h 1

#include "globals.hh"
#include "G4UImessenger.hh"
#include "G4String.hh"

class B1DetectorConstruction;
class G4UIdirectory;
class G4UIcmdWithADouble;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithAnInteger;

class B1DetectorConstructionMessenger : public G4UImessenger{
public:
  B1DetectorConstructionMessenger(B1DetectorConstruction *);
  ~B1DetectorConstructionMessenger();
  
  void SetNewValue(G4UIcommand *, G4String );
  
private:
  
  B1DetectorConstruction * Detector;
  G4UIdirectory          * Dir;
  
  G4UIcmdWithAnInteger   * SetMyVerbosityCmd;
  // 0 - none of my outputs
  // 1 - event level outputs
  
  
  G4UIcmdWithADouble     * SetAlphaDistanceCmd;
  // -1 no source (<0 in code), not working yet
  // other values are cm distance
  
  G4UIcmdWithADouble   * SetGammaHeightCmd;
  // 0 no source
  // 1...3 different positions
  
};

#endif