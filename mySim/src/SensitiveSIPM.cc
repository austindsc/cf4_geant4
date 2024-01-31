#include "SensitiveSIPM.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicsOrderedFreeVector.hh"
#include "G4RunManager.hh"

#include "CF4EventAction.hh"
#include "HelperFunctions.hh"
#include "MyGlobals.hh"

#include <filesystem>

SensitiveSIPM::SensitiveSIPM(G4String name) : G4VSensitiveDetector(name )
{ 
  fEventAction = (CF4EventAction *) G4RunManager::GetRunManager()->GetUserEventAction();
  fhc = CLHEP::h_Planck * CLHEP::c_light;
  fLastTrackID = -1; // so don't miss first track

  // adding PDE
  ovecPDE = new G4PhysicsFreeVector(false);
  
  G4double scaleX, scaleY;
  // using wavelengths, not energy, is okay. Do the efficiency call on my own
  std::string fname = "dataOptics/effective_internalPDE.txt";
  OH::LoadTxt_OrderedVector(fname, ovecPDE, MG::GetVerbosity(), scaleX = nm, scaleY = 1.);
  if(MG::GetVerbosity() > 1) {
    G4cout << "dumping PDE values from\n" << fname << G4endl; ovecPDE->DumpValues();

    G4cout << 
    "------------------------ SiPM Implementation: enforce Absorption in Si ------------------------\n"
    "using normal ProcessHits in sensitive detector. \n"
    "Must define ABSLENGTH as short in SiBulk. Then ProcessHits() checks if pre and post volumes are equal"
    "if they are equal it kills the track and triggers 'count PhotonIn()\n"
    "------------------------  ------------------------" << G4endl;
  }

  // G4cout << "PDE at 199.975 nm: " << ovecPDE->Value(199.975*nm) << G4endl;
  // exit(0);
  return;
}

SensitiveSIPM::~SensitiveSIPM()
{}

G4bool SensitiveSIPM::ProcessHits(G4Step * step, G4TouchableHistory *ROhist){
  G4Track * track = step->GetTrack();
  this->CountPhotonTouching(track->GetTrackID()); // if photon touches the surface of the SiPM
  
  // kill photon once inside Si (requires ABSLength short) and both pre and post steps are equal
  // this is option 2 from PDE study 
  G4VPhysicalVolume* physPreVolume = step->GetPreStepPoint()->GetPhysicalVolume();
  G4VPhysicalVolume* physPostVolume = step->GetPostStepPoint()->GetPhysicalVolume();
  if(physPreVolume == physPostVolume){
    track->SetTrackStatus(fStopAndKill);
    this->CountPhotonIn(step);
    return true;
  }
  return false;
}

// do a bit of logic here to avoid double counting twice reflected photons
inline G4bool SensitiveSIPM::CountPhotonTouching(G4int trackID){
  // G4cout << "in photon Touching" << G4endl;
  if(trackID == fLastTrackID) return false;
  else{
    fLastTrackID = trackID;
    fEventAction->AddTouchingPE();
    return true;
  }
}

inline G4bool SensitiveSIPM::CountPhotonIn(const G4Step * step){

  G4StepPoint * preStepPoint = step->GetPreStepPoint();
  G4ThreeVector pPhoton = preStepPoint->GetMomentum();
  G4double wlen = fhc/pPhoton.mag(); //wavelength in nanometers 

  fEventAction->AddTruthPE(); // photon entered
  if(MG::GetVerbosity() >= 2) { G4cout << "MC truth Hit! wl = " << wlen/nm << " nm" << G4endl; }

  // With effective internal Efficiency
  if(G4UniformRand() < ovecPDE->Value(wlen)){
    if(MG::GetVerbosity() >= 2)G4cout << "PDE hit! wl = " << wlen/nm << " nm" <<G4endl;
    fEventAction->AddDetPE();
  }
  return true;
}