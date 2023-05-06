#include "sensitiveSipm.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicsOrderedFreeVector.hh"
// #include "G"
#include "G4RunManager.hh"

#include "HelperFunctions.hh"
#include "myGlobals.hh"

#include <filesystem>

sensitiveSipm::sensitiveSipm(G4String name) : G4VSensitiveDetector(name )
{
  fEventAction = (B1EventAction *) G4RunManager::GetRunManager()->GetUserEventAction();

  // adding PDE
  ovecPDE = new G4PhysicsFreeVector(true);
  
  OH::LoadTxt_OrderedVector("data/SiPMeff_Ham_5V.dat", ovecPDE, MG::GetVerbosity(), 1, 1./100);
  //ovecPDE->DumpValues();
  //ovecPDE->SetSpline(false); // okay? large interpolation can cause errors
}

sensitiveSipm::~sensitiveSipm()
{}

G4bool sensitiveSipm::ProcessHits(G4Step * step, G4TouchableHistory *ROhist){

  G4Track * track = step->GetTrack();
  track->SetTrackStatus(fStopAndKill);
  
  G4StepPoint * preStepPoint = step->GetPreStepPoint(); // entering detector
  G4StepPoint * postStepPoint = step->GetPostStepPoint(); // leaving detector?
  
  G4ThreeVector posPhoton = preStepPoint->GetPosition();
  G4ThreeVector pPhoton = preStepPoint->GetMomentum();
  G4double wlen = (1239.8*eV/pPhoton.mag()) * nm; //wavelength in nanometers 
  
  if(MG::GetVerbosity() >= 2)G4cout << "MC Hit! wl = " << wlen/nm << " nm" << G4endl;
  fEventAction->AddmcPE(); 

  // With Efficiencies
  if(G4UniformRand() < ovecPDE->Value(wlen)){
    if(MG::GetVerbosity() >= 2)G4cout << "Raw hit! wl = " << wlen/nm << " nm" <<G4endl;
    fEventAction->AddPE();
  }
  
  // G4cout << "photon hit!: " << posPhoton << G4endl;
  
  
  return true;
}