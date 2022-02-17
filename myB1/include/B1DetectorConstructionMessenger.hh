#ifndef B1DetectorConstructionMessenger_h
#define B1DetectorConstructionMessenger_h 1

#include "globals.hh"
#include "G4UImessenger.hh"
#include "G4String.hh"

class B1DetectorConstruction;
class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithAnInteger;
class G4UIcmdWithADouble;
class G4UIcmdWithoutParameter;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWith3VectorAndUnit;

class B1DetectorConstructionMessenger : public G4UImessenger{
public:
  B1DetectorConstructionMessenger(B1DetectorConstruction *);
  ~B1DetectorConstructionMessenger();
  
  void SetNewValue(G4UIcommand *, G4String );
  
private:
  
  B1DetectorConstruction * Detector;
  
  G4UIdirectory          * Dir;
  G4UIcmdWithAString     * SetSomething;
  G4UIcmdWithADouble     * SetAlphaDistanceCmd;
  // -1 no source (<0 in code)
  // other values are cm distance
  
  G4UIcmdWithAnInteger   * SetGammaLocationCmd;
  // 0 no source
  // 1...3 different positions
  
  G4UIcmdWithADoubleAndUnit * SetCF4DensityCmd;
  // liqiuid or gas haha
  
};

#endif