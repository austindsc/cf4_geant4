/// \file CF4EventAction.cc
/// \brief Implementation of the CF4EventAction class

#include "CF4EventAction.hh"
#include "CF4RunAction.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"

#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "G4RootAnalysisManager.hh"

#include "G4GeneralParticleSource.hh"

#include <stdlib.h>

#include "MyGlobals.hh"


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CF4EventAction::CF4EventAction(CF4RunAction* runAction)
: G4UserEventAction(),
  fRunAction(runAction),
  fEdep(0.),
  fmcNPE(0)
{
  evMan = G4EventManager::GetEventManager();
} 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CF4EventAction::~CF4EventAction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CF4EventAction::BeginOfEventAction(const G4Event*)
{    
  G4int evt = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
  if((MG::GetVerbosity() == 1 || evMan->GetVerboseLevel() > 0) && (evt < 100 || evt % 1000 == 0)){
    G4cout << "\n-------- Start Event --------" << G4endl;
  }
  fEdep = 0.;
  ftouchPE = 0.;
  fmcNPE = 0;
  fNPE = 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CF4EventAction::EndOfEventAction(const G4Event*)
{   
  // not doing any event skipping or anything yet
  // G4cout << fEdep / keV << G4endl;
  // if(fEdep <= 10 * eV) return;

  // tracking some stuff for photon testing
  fRunAction->ftotalMCPE += fmcNPE;
  fRunAction->ftotalFired += fRunAction->fGenerator->fGun->GetNumberOfParticles();
  
  G4int evt = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
    if(fEdep > 0 || MG::GetVerbosity() == -1 || MG::GetVerbosity() > 1){
    G4AnalysisManager *man = G4AnalysisManager::Instance();
    man->FillNtupleIColumn(0, evt);
    man->FillNtupleDColumn(1, fEdep/keV);
    man->FillNtupleIColumn(2, fmcNPE);
    man->FillNtupleIColumn(3, fNPE);
    man->AddNtupleRow(0);
  }
  
  if((MG::GetVerbosity() >= 1 || evMan->GetVerboseLevel() > 0) && (evt < 100 || evt % 1000 == 0)){
    G4cout << "\n---- My Event Info #" << evt << "  ----\n\n" 
    << "total touched = " << ftouchPE << "\n"
    << "total MC PE = " << fmcNPE << "\n"
    << "total PE = " << fNPE << G4endl;
    G4cout << "CF4eventAction::EOEA fEdep = " << fEdep/keV << " keV" << G4endl;
    G4cout << "\n---- Event Complete ----\n" << G4endl;
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
