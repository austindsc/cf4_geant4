
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
  G4cout << "yoo austin we built the new class" << G4endl;
  
  Dir = new G4UIdirectory("/setDetector/");
  Dir->SetGuidance("Set run to run detector parameters (source height, cf4 density, etc)");
  
  SetAlphaDistanceCmd = new G4UIcmdWithADouble("/setDetector/alphaPosition", this);
  SetAlphaDistanceCmd->SetGuidance("Insert z distance (in cm) between SiPM face and alpha source. set val<0 for no Alpha source");
  SetAlphaDistanceCmd->SetParameterName("alphaPosition", false, true); // omittable, but has default value
  SetAlphaDistanceCmd->SetDefaultValue(det->GetAlphaHeight()); //default values in detectorconstruction constructor
  SetAlphaDistanceCmd->AvailableForStates(G4State_PreInit);

  SetCF4DensityCmd = new G4UIcmdWithADoubleAndUnit("/setDetector/cf4Density", this);
  SetCF4DensityCmd->SetGuidance("give CF4 density and associated units (probably use g/cm^3)");
  SetCF4DensityCmd->SetParameterName("cf4Density", false, true);
  SetCF4DensityCmd->SetDefaultValue(det->GetCF4Density());//default values in detectorconstruction constructor
  SetCF4DensityCmd->SetUnitCategory("Volumic Mass");
  SetCF4DensityCmd->AvailableForStates(G4State_PreInit);
  
  // LY stuff
  new G4UnitDefinition("LY per MeV", "MeVLY", "inv. energy Yield", 1/MeV);
  new G4UnitDefinition("LY per keV", "keVLY", "inv. energy Yield", 1/keV);
  
  SetLYCmd = new G4UIcmdWithADoubleAndUnit("/setDetector/cf4LY", this);
  SetLYCmd->SetGuidance("give LY and units (use MeVLY or keVLY)");
  SetLYCmd->SetParameterName("cf4LY", false, true);
  SetLYCmd->SetDefaultValue(det->GetLY());//default values in detectorconstruction constructor
  SetLYCmd->SetUnitCandidates("keVLY MeVLY"); 
  SetLYCmd->SetUnitCategory("inv. energy Yield");
  SetLYCmd->AvailableForStates(G4State_PreInit);
  
}

B1DetectorConstructionMessenger::~B1DetectorConstructionMessenger(){
  
  delete Dir;
  
  delete SetAlphaDistanceCmd;
}

void B1DetectorConstructionMessenger::SetNewValue(G4UIcommand *command, G4String newVal){
  
  // do something
  if(command == SetAlphaDistanceCmd){
    Detector->SetAlphaHeight(SetAlphaDistanceCmd->GetNewDoubleValue(newVal) * cm);
  } else if (command == SetCF4DensityCmd){
    Detector->SetCF4Density(SetCF4DensityCmd->GetNewDoubleValue(newVal));
  } else if (command == SetLYCmd){
    Detector->SetLY(SetLYCmd->GetNewDoubleValue(newVal));
  }
  
}








