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
/// \file cf4Chamber.cc
/// \brief Main program of the CF4 example

#include "CF4DetectorConstruction.hh"
#include "CF4ActionInitialization.hh"

#include "G4RunManagerFactory.hh"

#include "G4UImanager.hh"

// physics lists
#include "PhysicsList.hh"

#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

#include "G4SystemOfUnits.hh"

//xx physics list testing
#include "FTFP_BERT.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4OpticalParameters.hh"
#include "G4OpticalPhysics.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc,char** argv)
{

  // cout << USE_REFPROP << endl;
  // return 1;
  // #ifdef USE_REFPROP
  // exit(0);
  // #endif

  // Detect interactive mode (if no arguments) and define UI session
  G4UIExecutive* ui = 0;
  if ( argc == 1 ) { ui = new G4UIExecutive(argc, argv);  }
  
  // Construct the default run manager
  G4RunManager * runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
  runManager->SetVerboseLevel(0);

  // Detector construction
  runManager->SetUserInitialization(new CF4DetectorConstruction());

  // Physics list
  // G4VUserPhysicsList * physList = new myPhysList();
  // runManager->SetUserInitialization(physList);
  // runManager->SetUserInitialization(new PhysicsList());

  // xx done testing
  G4VModularPhysicsList* physicsList = new FTFP_BERT;
  physicsList->ReplacePhysics(new G4EmStandardPhysics_option4());

  G4OpticalPhysics* opticalPhysics = new G4OpticalPhysics();
  auto opticalParams               = G4OpticalParameters::Instance();

  opticalParams->SetWLSTimeProfile("delta");

  opticalParams->SetScintTrackSecondariesFirst(true);

  opticalParams->SetCerenkovMaxPhotonsPerStep(100);
  opticalParams->SetCerenkovMaxBetaChange(10.0);
  opticalParams->SetCerenkovTrackSecondariesFirst(true);

  physicsList->RegisterPhysics(opticalPhysics);
  runManager->SetUserInitialization(physicsList);
  // xx done testing

  
  // User action initialization
  runManager->SetUserInitialization(new CF4ActionInitialization());
  runManager->SetVerboseLevel(0);
  
  // Initialize visualization
  G4VisManager* visManager = new G4VisExecutive();
  visManager->SetVerboseLevel(0);
  visManager->Initialize();

  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();
  UImanager->SetVerboseLevel(0);

  // Process macro or start UI session
  if ( ! ui ) { 
    // batch mode
    G4cout << "\n\n---------------------- Script Settings ----------------------" << G4endl;
    // G4String command = "/control/execute /Users/austindestcroix/Desktop/school_holders/projects/sbc/geant4_cf4";
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command+fileName);
  }
  else {
    // interactive mode
    G4cout << "\n\n---------------------- Script Settings ----------------------\n" << G4endl;
    UImanager->ApplyCommand("/control/execute ../mySim/defaultRun.mac");

    ui->SessionStart();
    delete ui;
  }

  // Job termination
  // Free the store: user actions, physics_list and detector_description are
  // owned and deleted by the run manager, so they should not be deleted 
  // in the main() program !
  delete visManager;
  delete runManager;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....
