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
/// \file CF4RunAction.cc
/// \brief Implementation of the CF4RunAction class

#include "CF4RunAction.hh"
#include "CF4PrimaryGeneratorAction.hh"
#include "CF4DetectorConstruction.hh"
// #include "CF4Run.hh"

#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4AccumulableManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

#include "G4GeneralParticleSource.hh"

#include "G4AnalysisManager.hh"
// #include "G4AnalysisUtilities.hh"
#include "G4RootAnalysisManager.hh"

// #include "G4Random.hh"
#include "Randomize.hh"
#include "MyGlobals.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CF4RunAction::CF4RunAction(CF4PrimaryGeneratorAction * genny)
: G4UserRunAction(),
  fEdep(0.),
  fEdep2(0.),
  favgEdep(0.)
{ 
  fGenerator = genny; // for photon testing if multiple photons are shot
  ftotalMCPE = 0;
  ftotalFired = 0;
  
  G4AnalysisManager * man = G4AnalysisManager::Instance();
  man->SetDefaultFileType("root");
  man->SetFileName("cf4_outputStock");
  man->CreateNtuple("simOutput", "simOutput");
  // but if you don't do this, you can't name the file from the macro
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CF4RunAction::~CF4RunAction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CF4RunAction::BeginOfRunAction(const G4Run*)
{ 
  ftotalMCPE = 0;
  ftotalFired = 0;
  // inform the runManager to save random number seed
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);
  
  G4int seed=time(0);
  G4Random::setTheSeed(seed);

  // reset accumulables to their initial values
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Reset();
  
  // TRandom * rand = new TRandom3();
  G4AnalysisManager *man = G4AnalysisManager::Instance();
  
  man->CreateNtupleIColumn("fNEvent");        //0
  man->CreateNtupleDColumn("fEdepInCyl_kev"); //1
  man->CreateNtupleIColumn("fTouchPE");       //2 - MC photons touching Si bulk (consistent with previous fmcNPE except for reflections off window, SiO2 layer)
  man->CreateNtupleIColumn("fNPE");           //3 - photons detected after internal PDE (consisten with previous fNPE variable)
  man->CreateNtupleIColumn("fEnterPE");       //4 - MC photons entering Si bulk (new variable)
  man->FinishNtuple(0);

  man->OpenFile();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CF4RunAction::EndOfRunAction(const G4Run* run)
{
  G4int nofEvents = run->GetNumberOfEvent();
  if (nofEvents == 0) return;
        
  // Print
  if (IsMaster()) {
    G4cout
     << G4endl
     << "--------------------End of Global Run-----------------------";
  }
  else {
    G4cout
     << G4endl
     << "--------------------End of Local Run------------------------";
  }
  
  G4cout << "\nFinished run of " << nofEvents << " events" << G4endl;

  // getting the ntuple
  G4AnalysisManager * man = G4AnalysisManager::Instance();

  if(MG::GetVerbosity() > 0) {
    
    G4cout << ftotalFired << " particles fired, " << ftotalMCPE << " photons entered sipm" << G4endl;
    G4cout << (double) ftotalMCPE/ftotalFired << G4endl;

  }
  
   man->Write();
   man->CloseFile();

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// void CF4RunAction::AddEdep(G4double edep)
// {
//   fEdep  += edep;
//   fEdep2 += edep*edep;
// }


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

