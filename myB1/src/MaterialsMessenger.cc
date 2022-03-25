
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
  SetMediumDensityCmd->SetGuidance("give medium density and associated units (probably use g/cm3)");
  SetMediumDensityCmd->SetParameterName("mediumDensity", false, true);
  SetMediumDensityCmd->SetDefaultValue(dmat->fmediumDensity);//default values in detectorconstruction constructor
  SetMediumDensityCmd->SetUnitCategory("Volumic Mass");
  SetMediumDensityCmd->AvailableForStates(G4State_PreInit);
  
  // LY stuff
  new G4UnitDefinition("LY per MeV", "MeVLY", "inv. energy Yield", 1/MeV);
  new G4UnitDefinition("LY per keV", "keVLY", "inv. energy Yield", 1/keV);
  
  SetMediumLYCmd = new G4UIcmdWithADoubleAndUnit("/setMaterial/mediumLY", this);
  SetMediumLYCmd->SetGuidance("give LY and units (use MeVLY or keVLY)");
  SetMediumLYCmd->SetParameterName("mediumLY", false, true);
  SetMediumLYCmd->SetDefaultValue(dmat->fmediumLY);//default values in detectorconstruction constructor
  SetMediumLYCmd->SetUnitCandidates("keVLY MeVLY"); 
  SetMediumLYCmd->SetUnitCategory("inv. energy Yield");
  SetMediumLYCmd->AvailableForStates(G4State_PreInit);
  
  SetMediumTempCmd = new G4UIcmdWithADoubleAndUnit("/setMaterial/mediumTemp", this);
  SetMediumTempCmd->SetGuidance("temp (if it matters), kelvin");
  SetMediumTempCmd->SetParameterName("mediumTemp", false, true);
  SetMediumTempCmd->SetDefaultValue(dmat->fmediumTemp);//default values in detectorconstruction constructor
  SetMediumTempCmd->SetUnitCandidates("kelvin"); 
  SetMediumTempCmd->AvailableForStates(G4State_PreInit);
  
  SetMediumPressureCmd = new G4UIcmdWithADoubleAndUnit("/setMaterial/mediumPressure", this);
  SetMediumPressureCmd->SetGuidance("pressure (if it matters), bar, atm");
  SetMediumPressureCmd->SetParameterName("mediumPressure", false, true);
  SetMediumPressureCmd->SetDefaultValue(dmat->fmediumPressure);//default values in detectorconstruction constructor
  SetMediumPressureCmd->SetUnitCandidates("bar atm"); 
  SetMediumPressureCmd->AvailableForStates(G4State_PreInit);
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
    G4cout << "Setting the medium to " << newVal << G4endl;
  }
  if (command == SetMediumDensityCmd){
    fdmat->fmediumDensity = SetMediumDensityCmd->GetNewDoubleValue(newVal);
    G4cout << "Changing medium Density to = " << SetMediumDensityCmd->GetNewDoubleValue(newVal)/(g/cm3) << " g/cm3" << G4endl;
  }
  if (command == SetMediumLYCmd){
    fdmat->SetMediumLY(SetMediumLYCmd->GetNewDoubleValue(newVal));
    G4cout << "Changing LY to = " << SetMediumLYCmd->GetNewDoubleValue(newVal)/MeV << " photon/MeV" << G4endl;
  }
  if (command == SetMediumTempCmd){
    fdmat->fmediumTemp = SetMediumTempCmd->GetNewDoubleValue(newVal);
    G4cout << "Setting Temperature to = " << SetMediumTempCmd->GetNewDoubleValue(newVal)/kelvin << " K" << G4endl;
  }
  if (command == SetMediumPressureCmd){
    fdmat->fmediumPressure = SetMediumPressureCmd->GetNewDoubleValue(newVal);
    G4cout << "Setting Pressure to = " << SetMediumPressureCmd->GetNewDoubleValue(newVal)/bar << " bar" << G4endl;
  }
  return;
}








