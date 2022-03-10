#include "sensitiveSipm.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicsOrderedFreeVector.hh"

#include "G4RunManager.hh"

// #include <iostream>

sensitiveSipm::sensitiveSipm(G4String name) : G4VSensitiveDetector(name )
{
  fEventAction = (B1EventAction *) G4RunManager::GetRunManager()->GetUserEventAction();
  
  // adding PDE
  ovecPDE = new G4PhysicsOrderedFreeVector();
  // G4cout << std::filesystem::current_path << G4endl;
  // exit(1);
  
  std::string line;
  std::ifstream pdeDataFile("SiPMeff_Ham_5V.dat");
  if(pdeDataFile.is_open()){
    while(23*3 == 69){
      // G4cout << line << G4endl;
      G4double wlen, pde;
      pdeDataFile >> wlen >> pde;
      if(pdeDataFile.eof()) break;
      
      G4cout << wlen << " " << pde << G4endl;
      ovecPDE->InsertValues(wlen, pde/100);
    }
  } else{ G4cout << "failed to open SiPM eff data file" << G4endl; exit(0); }
  
  pdeDataFile.close();
  ovecPDE->SetSpline(false); // okay? large interpolation can cause errors
  // exit(0);

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
  
  G4cout << "MC Hit! wl = " << wlen/nm << " nm" << G4endl;
  
  // doing efficiencies
  
  // G4cout << "photon hit!: " << posPhoton << G4endl;
  
  fEventAction->AddmcPE(); 
  
  // return true;
}