#ifndef CF4DetectorConstructionMessenger_h
#define CF4DetectorConstructionMessenger_h 1

#include "globals.hh"
#include "G4UImessenger.hh"
#include "G4String.hh"

class CF4DetectorConstruction;
class G4UIdirectory;
class G4UIcmdWithADouble;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithAString;
class G4UIcmdWithAnInteger;
class G4UIcmdWithABool;

class CF4DetectorConstructionMessenger : public G4UImessenger{
public:
  CF4DetectorConstructionMessenger(CF4DetectorConstruction *);
  ~CF4DetectorConstructionMessenger();
  
  void SetNewValue(G4UIcommand *, G4String );
  
private:
  
  CF4DetectorConstruction * Detector;
  G4UIdirectory          * Dir;
  
  G4UIcmdWithAnInteger   * SetMyVerbosityCmd;
  // 0 - none of my outputs
  // 1 - event level outputs
  
  G4UIcmdWithADouble     * SetAlphaDistanceCmd;
  // -1 no source (<0 in code), not working yet
  // other values are cm distance
  
  G4UIcmdWithADouble    * SetGammaHeightCmd;
  // in cm, Z above SiPM face
  // this turns on gammaStatus in code
  
  G4UIcmdWithABool      * SetCollimatorStatusCmd;
  // true/false
  
  G4UIcmdWithADouble    * SetGammaThetaMaxCmd;
  // 0-180 (degrees)
  
  G4UIcmdWithAString    * SetGammaSourceCmd;
  // /setDetector/gammaSource
  // isoCs137
  // isoCo60
  // isoCo57
  // Cs137 (angle biasing, not complete decay information)
  // Co60 (angle biasing, not complete decay information)
  // Co57 (angle biasing, not complete decay information)
  
  // 0 no source
  // 1...3 different positions
  
};

#endif