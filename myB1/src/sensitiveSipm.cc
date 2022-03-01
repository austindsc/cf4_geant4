#include "sensitiveSipm.hh"

sensitiveSipm::sensitiveSipm(G4String name) : G4VSensitiveDetector(name )
{}

sensitiveSipm::~sensitiveSipm()
{}

G4bool sensitiveSipm::ProcessHits(G4Step * step, G4TouchableHistory *ROhist){

  G4Track * track = step->GetTrack();
  
  track->SetTrackStatus(fStopAndKill);
  
  G4StepPoint * preStepPoint = step->GetPreStepPoint(); // entering detector
  G4StepPoint * postStepPoint = step->GetPostStepPoint(); // leaving detector?
  
  G4ThreeVector posPhoton = preStepPoint->GetPosition();
  
  G4cout << "photon hit!: " << posPhoton << G4endl;
  
  // return true;
}