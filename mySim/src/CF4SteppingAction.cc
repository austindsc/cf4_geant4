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
/// \file CF4SteppingAction.cc
/// \brief Implementation of the CF4SteppingAction class

#include "CF4SteppingAction.hh"
#include "CF4EventAction.hh"
#include "CF4DetectorConstruction.hh"

#include "G4Step.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4LogicalVolume.hh"

#include "G4OpticalPhoton.hh"
#include "SensitiveSIPM.hh"


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CF4SteppingAction::CF4SteppingAction(CF4EventAction* eventAction)
: G4UserSteppingAction(),
  fEventAction(eventAction),
  fScoringVolume(0)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CF4SteppingAction::~CF4SteppingAction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CF4SteppingAction::UserSteppingAction(const G4Step* step)
{
  if (!fScoringVolume) { 
    const CF4DetectorConstruction* detectorConstruction
      = static_cast<const CF4DetectorConstruction*>
        (G4RunManager::GetRunManager()->GetUserDetectorConstruction());
    fScoringVolume = detectorConstruction->GetScoringVolume();   
  }

  // ------------------------------------------
  static G4ParticleDefinition* opticalphoton = G4OpticalPhoton::OpticalPhotonDefinition();

  // Handle optical photon boundary interactions BEFORE other processes
  if(step->GetTrack()->GetParticleDefinition() == opticalphoton){
    
    // Check if photon is crossing into a SiPM volume (has sensitive detector)
    G4VPhysicalVolume* preVolume = step->GetPreStepPoint()->GetPhysicalVolume();
    G4VPhysicalVolume* postVolume = step->GetPostStepPoint()->GetPhysicalVolume();
    
    if (preVolume != postVolume && postVolume) {
      G4LogicalVolume* postLogical = postVolume->GetLogicalVolume();
      if (postLogical && postLogical->GetSensitiveDetector()) {
        
        // Check if it's a SensitiveSIPM
        SensitiveSIPM* sipmSD = dynamic_cast<SensitiveSIPM*>(postLogical->GetSensitiveDetector());
        if (sipmSD) {
          // Let the SensitiveSIPM handle the surface optics
          if (sipmSD->HandlePhotonAtBoundary(step)) {
            return; // SiPM handled the photon (reflected/transmitted/absorbed)
          }
        }
      }
    }
    
    return; // Don't process optical photons for energy deposition
  }

  // ------------------------------------------
  // Handle non-optical particles for energy deposition
  
  // get volume of the current step
  G4LogicalVolume* volume = step->GetPreStepPoint()->GetTouchableHandle()
      ->GetVolume()->GetLogicalVolume();
      
  // check if we are in scoring volume
  if (volume != fScoringVolume) return;

  // collect energy deposited in this step
  G4double edepStep = step->GetTotalEnergyDeposit();
  
  fEventAction->AddEdep(edepStep);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

