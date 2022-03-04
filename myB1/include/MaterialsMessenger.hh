#ifndef MATERIALSMESSENGER_h
#define MATERIALSMESSENGER_h 1

#include "globals.hh"
#include "G4UImessenger.hh"
#include "G4String.hh"

class DetectorMaterials;
class G4UIdirectory;
class G4UIcmdWithADouble;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithAString;

class MaterialsMessenger : public G4UImessenger{
public:
  MaterialsMessenger(DetectorMaterials *);
  ~MaterialsMessenger();
  
  void SetNewValue(G4UIcommand *, G4String );
  
private:
  
  DetectorMaterials     * fdmat;
  G4UIdirectory          * Dir;
  
  G4UIcmdWithAString     * SetDetectionMediumCmd;
  G4UIcmdWithADoubleAndUnit * SetMediumDensityCmd;
  G4UIcmdWithADoubleAndUnit * SetMediumLYCmd;
  
  
};

#endif