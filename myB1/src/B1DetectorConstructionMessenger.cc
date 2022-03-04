
#include "G4UIdirectory.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"

#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

#include "B1DetectorConstruction.hh"
#include "B1DetectorConstructionMessenger.hh"


B1DetectorConstructionMessenger::B1DetectorConstructionMessenger(B1DetectorConstruction * det)
:Detector(det)
{
  
  Dir = new G4UIdirectory("/setDetector/");
  Dir->SetGuidance("Set run to run detector parameters (source height, cf4 density, etc)");
  
  SetAlphaDistanceCmd = new G4UIcmdWithADouble("/setDetector/alphaPosition", this);
  SetAlphaDistanceCmd->SetGuidance("Insert z distance (in cm) between SiPM face and alpha source. set val<0 for no Alpha source");
  SetAlphaDistanceCmd->SetParameterName("alphaPosition", false, true); // omittable, but has default value
  SetAlphaDistanceCmd->SetDefaultValue(det->GetAlphaHeight()); //default values in detectorconstruction constructor
  SetAlphaDistanceCmd->AvailableForStates(G4State_PreInit);  
}

B1DetectorConstructionMessenger::~B1DetectorConstructionMessenger(){
  
  delete Dir;
  
  delete SetAlphaDistanceCmd;
}

void B1DetectorConstructionMessenger::SetNewValue(G4UIcommand *command, G4String newVal){
  
  // do something
  if(command == SetAlphaDistanceCmd){
    Detector->SetAlphaHeight(SetAlphaDistanceCmd->GetNewDoubleValue(newVal) * cm);
  }
  
}








