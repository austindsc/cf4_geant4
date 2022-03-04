
#include "G4UIdirectory.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"

#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

#include "MaterialsMessenger.hh"
#include "DetectorMaterials.hh"

MaterialsMessenger::MaterialsMessenger(DetectorMaterials * dmat)
:fdmat(dmat)
{
  
  Dir = new G4UIdirectory("/setMaterial/");
  Dir->SetGuidance("Set detector medium, density and LY");
  
  SetDetectionMediumCmd = new G4UIcmdWithAString("/setMaterial/detectionMedium", this);
  SetDetectionMediumCmd->SetGuidance("Select detection medium. 'gCF4', 'lCF4', 'gAr'");
  SetDetectionMediumCmd->SetParameterName("medium", false, true);
  SetDetectionMediumCmd->SetDefaultValue("gCF4");
  SetDetectionMediumCmd->AvailableForStates(G4State_PreInit);
  
  SetMediumDensityCmd = new G4UIcmdWithADoubleAndUnit("/setMaterial/mediumDensity", this);
  SetMediumDensityCmd->SetGuidance("give medium density and associated units (probably use g/cm^3)");
  SetMediumDensityCmd->SetParameterName("mediumDensity", false, true);
  SetMediumDensityCmd->SetDefaultValue(dmat->GetMediumDensity());//default values in detectorconstruction constructor
  SetMediumDensityCmd->SetUnitCategory("Volumic Mass");
  SetMediumDensityCmd->AvailableForStates(G4State_PreInit);
  
  // LY stuff
  new G4UnitDefinition("LY per MeV", "MeVLY", "inv. energy Yield", 1/MeV);
  new G4UnitDefinition("LY per keV", "keVLY", "inv. energy Yield", 1/keV);
  
  SetMediumLYCmd = new G4UIcmdWithADoubleAndUnit("/setMaterial/mediumLY", this);
  SetMediumLYCmd->SetGuidance("give LY and units (use MeVLY or keVLY)");
  SetMediumLYCmd->SetParameterName("mediumLY", false, true);
  SetMediumLYCmd->SetDefaultValue(dmat->GetMediumLY());//default values in detectorconstruction constructor
  SetMediumLYCmd->SetUnitCandidates("keVLY MeVLY"); 
  SetMediumLYCmd->SetUnitCategory("inv. energy Yield");
  SetMediumLYCmd->AvailableForStates(G4State_PreInit);
}

MaterialsMessenger::~MaterialsMessenger(){
  
  delete Dir;
  delete SetDetectionMediumCmd;
  delete SetMediumDensityCmd;
  delete SetMediumLYCmd;
}

void MaterialsMessenger::SetNewValue(G4UIcommand *command, G4String newVal){
  
  if(command == SetDetectionMediumCmd){
    fdmat->SetDetectionMedium(newVal);
  } else if (command == SetMediumDensityCmd){
    fdmat->SetMediumDensity(SetMediumDensityCmd->GetNewDoubleValue(newVal));
  } else if (command == SetMediumLYCmd){
    fdmat->SetMediumLY(SetMediumLYCmd->GetNewDoubleValue(newVal));
  }

}








