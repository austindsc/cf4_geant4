
#include "G4UIdirectory.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithABool.hh"

#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

#include "CF4DetectorConstruction.hh"
#include "CF4DetectorConstructionMessenger.hh"
#include "SiPMSurfaceWLS.hh"

#include "MyGlobals.hh"


CF4DetectorConstructionMessenger::CF4DetectorConstructionMessenger(CF4DetectorConstruction * det)
:Detector(det)
{
  
  Dir = new G4UIdirectory("/setDetector/");
  Dir->SetGuidance("Set run to run detector parameters (source height, cf4 density, etc)");
  
  SetMyVerbosityCmd = new G4UIcmdWithAnInteger("/setDetector/myVerbosity", this);
  SetMyVerbosityCmd->SetDefaultValue(MG::GetVerbosity()); //default values in detectorconstruction constructor
  SetMyVerbosityCmd->SetParameterName("myVerbosity", false, true);
  SetMyVerbosityCmd->AvailableForStates(G4State_PreInit);
  
  SetAlphaDistanceCmd = new G4UIcmdWithADouble("/setDetector/alphaHeight", this);
  SetAlphaDistanceCmd->SetGuidance("Insert z distance (in cm) between SiPM face and alpha source. set val<0 for no Alpha source");
  SetAlphaDistanceCmd->SetParameterName("alphaHeight", false, true); // omittable, but has default value
  SetAlphaDistanceCmd->SetDefaultValue(det->GetAlphaHeight()); //default values in detectorconstruction constructor
  SetAlphaDistanceCmd->AvailableForStates(G4State_PreInit); 

  SetGammaHeightCmd = new G4UIcmdWithADouble("/setDetector/gammaHeight", this);
  SetGammaHeightCmd->SetGuidance("Insert z distance (in cm) from SiPM height to gamma height (outside vacuum chamber). -Z is below SiPM");
  SetGammaHeightCmd->SetParameterName("gammaHeight", false, true); // omittable, but has default value
  SetGammaHeightCmd->SetDefaultValue(det->GetGammaHeight()); //default values in detectorconstruction constructor
  SetGammaHeightCmd->AvailableForStates(G4State_PreInit);  
  
  SetGammaSourceCmd = new G4UIcmdWithAString("/setDetector/gammaSource", this);
  SetGammaSourceCmd->SetParameterName("gammaSource", false, true);
  SetGammaSourceCmd->SetDefaultValue(det->GetGammaSourceStr());
  SetGammaSourceCmd->AvailableForStates(G4State_PreInit);
  
  SetCollimatorStatusCmd = new G4UIcmdWithABool("/setDetector/useCollimator", this);
  SetCollimatorStatusCmd->SetParameterName("useCollimator", false, true);
  SetCollimatorStatusCmd->SetDefaultValue(det->GetCollStatus());
  SetCollimatorStatusCmd->AvailableForStates(G4State_PreInit);

  SetLongPassFilterStatusCmd = new G4UIcmdWithABool("/setDetector/useLPFilter", this);
  SetLongPassFilterStatusCmd->SetParameterName("useLPFilter", true, true);
  SetLongPassFilterStatusCmd->SetDefaultValue(det->GetFilterStatus());
  SetLongPassFilterStatusCmd->AvailableForStates(G4State_PreInit);
  
  SetGammaThetaMaxCmd = new G4UIcmdWithADouble("/setDetector/GammaThetaMax", this);
  SetGammaThetaMaxCmd->SetParameterName("GammaThetaMax", false, true);
  SetGammaThetaMaxCmd->SetDefaultValue(det->GetGammaThetaMax());
  SetGammaThetaMaxCmd->AvailableForStates(G4State_PreInit);
  
  SetVoltageCmd = new G4UIcmdWithADouble("/setDetector/setVoltage", this);
  SetVoltageCmd->SetGuidance("Set SiPM bias voltage");
  SetVoltageCmd->SetParameterName("setVoltage", false);
  SetVoltageCmd->SetDefaultValue(4.0);
  SetVoltageCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  SetAbsorbingWallsCmd = new G4UIcmdWithABool("/setDetector/useAbsorbingWalls", this);
  SetAbsorbingWallsCmd->SetGuidance("Apply absorbing surface (paper) to CF4 cylinder walls");
  SetAbsorbingWallsCmd->SetParameterName("useAbsorbingWalls", false, true);
  SetAbsorbingWallsCmd->SetDefaultValue(true);
  SetAbsorbingWallsCmd->AvailableForStates(G4State_PreInit);
}

CF4DetectorConstructionMessenger::~CF4DetectorConstructionMessenger(){
  
  delete Dir;
  delete SetMyVerbosityCmd;
  delete SetAlphaDistanceCmd;
  delete SetGammaHeightCmd;
  delete SetGammaSourceCmd;
  delete SetVoltageCmd;
  delete SetAbsorbingWallsCmd;
}

void CF4DetectorConstructionMessenger::SetNewValue(G4UIcommand *command, G4String newVal){
  
  // do something
  if(command == SetMyVerbosityCmd){
    MG::SetVerbosity((int) SetMyVerbosityCmd->GetNewIntValue(newVal));
    G4cout << "Setting MyVerbosity to " << SetMyVerbosityCmd->GetNewIntValue(newVal) << G4endl;
  }
  if(command == SetAlphaDistanceCmd){
    Detector->SetAlphaHeight(SetAlphaDistanceCmd->GetNewDoubleValue(newVal) * cm);
  }
  if(command == SetGammaHeightCmd){
    Detector->SetGammaHeight(SetGammaHeightCmd->GetNewDoubleValue(newVal) * cm);
  }
  if(command == SetGammaSourceCmd){
    Detector->SetGammaSourceStr(newVal);
  }
  if(command == SetLongPassFilterStatusCmd){
    Detector->SetFilterStatus(newVal);
  }
  if(command == SetCollimatorStatusCmd){
    Detector->SetCollStatus(SetCollimatorStatusCmd->GetNewBoolValue(newVal));
  }
  if(command == SetGammaThetaMaxCmd){
    Detector->SetGammaThetaMax(SetGammaThetaMaxCmd->GetNewDoubleValue(newVal) * deg);
  }
  if(command == SetVoltageCmd){
    G4double voltage = SetVoltageCmd->GetNewDoubleValue(newVal);
    G4cout << "Setting SiPM voltage to " << voltage << " V" << G4endl;
    // Make voltage available to SiPMSurfaceWLS
    if (SiPMSurfaceWLS::GetInstance()) {
      SiPMSurfaceWLS::GetInstance()->SetVoltage(voltage);
    }
  }
  if(command == SetAbsorbingWallsCmd){
    DETECTORSETUP::SetAbsorbingStatus(SetAbsorbingWallsCmd->GetNewBoolValue(newVal));
    G4cout << "Setting absorbing walls to " << newVal << G4endl;
  }

}








