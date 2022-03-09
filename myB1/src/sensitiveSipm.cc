#include "sensitiveSipm.hh"

#include "G4RunManager.hh"

sensitiveSipm::sensitiveSipm(G4String name) : G4VSensitiveDetector(name )
{
  fEventAction = (B1EventAction *) G4RunManager::GetRunManager()->GetUserEventAction();
  
}

sensitiveSipm::~sensitiveSipm()
{}

G4bool sensitiveSipm::ProcessHits(G4Step * step, G4TouchableHistory *ROhist){

  G4Track * track = step->GetTrack();
  
  track->SetTrackStatus(fStopAndKill);
  
  G4StepPoint * preStepPoint = step->GetPreStepPoint(); // entering detector
  G4StepPoint * postStepPoint = step->GetPostStepPoint(); // leaving detector?
  
  G4ThreeVector posPhoton = preStepPoint->GetPosition();
  
  // G4cout << "photon hit!: " << posPhoton << G4endl;
  
  fEventAction->AddmcPE(); 
  
  // return true;
}