#include "SensitiveSIPM.hh"
#include "G4SystemOfUnits.hh"
#include "G4RunManager.hh"
#include "Randomize.hh"
#include "G4OpticalPhoton.hh"

#include "CF4EventAction.hh"
#include "MyGlobals.hh"


SensitiveSIPM::SensitiveSIPM(G4String name) : G4VSensitiveDetector(name) { 
  fEventAction = (CF4EventAction *) G4RunManager::GetRunManager()->GetUserEventAction();
  fhc = CLHEP::h_Planck * CLHEP::c_light;
  fLastTrackID = -1;
  
  G4cout << "SensitiveSIPM: Initialized (surface optics handled by SiPMSurfaceWLS)" << G4endl;
}

SensitiveSIPM::~SensitiveSIPM()
{}

//==============================================================================
// Simplified ProcessHits - just count photons that make it through surface processing
//==============================================================================
G4bool SensitiveSIPM::ProcessHits(G4Step * step, G4TouchableHistory *) {
    
  // none of this gets called anymore. this is just a wrapper class i guess.
  // and it lets us double check we are in sensitive volume (from other location)

    // Only process optical photons
    if (step->GetTrack()->GetParticleDefinition() != G4OpticalPhoton::OpticalPhoton()) {
        return false;
    }
    // Kill the photon track since it's been detected
    step->GetTrack()->SetTrackStatus(fStopAndKill);
    if(MG::GetVerbosity() >= 2) {
        G4cout << "SensitiveSIPM::ProcessHits() Photon detected and track killed." << G4endl;
    }
    return true;
    
    // // Note: Surface optics (reflection/transmission) is now handled by SiPMSurfaceWLS process
    // // This ProcessHits only gets called for photons that survive surface processing
    
    // // Count the photon - first check for multiple hits from same track
    // if (CountPhotonTouching(step->GetTrack()->GetTrackID())) {
    //     // This is a new photon track touching the SiPM
    //     // do nothing for now - just count it
    // }
    
    // return false;
}

//==============================================================================
// Check if this is a new track touching the SiPM
//==============================================================================
G4bool SensitiveSIPM::CountPhotonTouching(G4int trackID) {
    if (trackID != fLastTrackID) {
        fLastTrackID = trackID;
        fEventAction->AddTouchingPE(); // photon touching SiPM (for the first time)
        return true; // New track
    }
    return false; // Same track as last time
}

//==============================================================================
// Count photon (placeholder for detection efficiency, PDE, etc.)
//==============================================================================
G4bool SensitiveSIPM::CountPhotonIn(G4double wavelength, G4double angleDeg) {
    fEventAction->AddTransmittedPE(); // photon made it into SiPM    
    return true;
}

G4bool SensitiveSIPM::CountPhotonDetected(G4double wavelength, G4double angleDeg) {
    fEventAction->AddDetPE(); // photon detected
    return true;
}