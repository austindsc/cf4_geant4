
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"

#include "B1DetectorConstruction.hh"
#include "B1DetectorConstructionMessenger.hh"


B1DetectorConstructionMessenger::B1DetectorConstructionMessenger(B1DetectorConstruction * det)
:Detector(det)
{
  G4cout << "yoo austin we built the new class" << G4endl;
  // G4Exception("B1DetConstMessager",
  // "Austin's warning message", FatalException, "uhm");
}

B1DetectorConstructionMessenger::~B1DetectorConstructionMessenger(){
  
}

void B1DetectorConstructionMessenger::SetNewValue(G4UIcommand *command, G4String newVal){
  
  //do something
}