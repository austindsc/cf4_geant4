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
    
    // Only process optical photons
    if (step->GetTrack()->GetParticleDefinition() != G4OpticalPhoton::OpticalPhoton()) {
        return false;
    }
    
    // Note: Surface optics (reflection/transmission) is now handled by SiPMSurfaceWLS process
    // This ProcessHits only gets called for photons that survive surface processing
    
    // Count the photon - first check for multiple hits from same track
    if (CountPhotonTouching(step->GetTrack()->GetTrackID())) {
        // This is a new photon track touching the SiPM
        if (CountPhotonIn(step)) {
            if (MG::GetVerbosity() >= 2) {
                G4double wavelength = fhc / step->GetTrack()->GetMomentum().mag() / CLHEP::nm;
                G4cout << "SensitiveSIPM: Counting photon (λ=" << wavelength << " nm)" << G4endl;
            }
            return true;
        }
    }
    
    return false;
}

//==============================================================================
// Check if this is a new track touching the SiPM
//==============================================================================
G4bool SensitiveSIPM::CountPhotonTouching(G4int trackID) {
    if (trackID != fLastTrackID) {
        fLastTrackID = trackID;
        return true; // New track
    }
    return false; // Same track as last time
}

//==============================================================================
// Count photon (placeholder for detection efficiency, PDE, etc.)
//==============================================================================
G4bool SensitiveSIPM::CountPhotonIn(const G4Step * step) {
    // For now, count all photons that make it here
    // Future: apply PDE, quantum efficiency, etc.
    
    G4double wavelength = fhc / step->GetTrack()->GetMomentum().mag() / CLHEP::nm;
    
    // Simple detection - just increment the counter
    if (fEventAction) {
        fEventAction->AddDetPE();
        if (MG::GetVerbosity() >= 1) {
            G4cout << "SensitiveSIPM: Detected photon at wavelength " << wavelength << " nm" << G4endl;
        }
    }
    
    return true;
}
