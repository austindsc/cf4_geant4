//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
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

#include "myGlobals.hh"


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
    if(fEdep > 0 || MG::GetVerbosity() == -1 || MG::GetVerbosity() > 0){
    G4AnalysisManager *man = G4AnalysisManager::Instance();
    man->FillNtupleIColumn(0, evt);
    man->FillNtupleDColumn(1, fEdep/keV);
    man->FillNtupleIColumn(2, fmcNPE);
    man->FillNtupleIColumn(3, fNPE);
    man->AddNtupleRow(0);
  }
  
  if((MG::GetVerbosity() >= 1 || evMan->GetVerboseLevel() > 0) && (evt < 100 || evt % 1000 == 0)){
    G4cout << "\n---- My Event Info #" << evt << "  ----\n\n" 
    << "total MC PE = " << fmcNPE << "\n"
    << "total PE = " << fNPE << G4endl;
    G4cout << "CF4eventAction::EOEA fEdep = " << fEdep/keV << " keV" << G4endl;
    G4cout << "\n---- Event Complete ----\n" << G4endl;
  }
}

 void CF4EventAction::AddEdep(G4double edep){ fEdep += edep; }
 void CF4EventAction::AddmcPE(){fmcNPE++;}
 void CF4EventAction::AddPE(){fNPE++;}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
