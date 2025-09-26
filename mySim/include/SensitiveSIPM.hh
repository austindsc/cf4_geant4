#ifndef SensitiveSIPM_H
#define SensitiveSIPM_H

#include "G4VSensitiveDetector.hh"

class G4UniformRand;
class CF4EventAction;

class SensitiveSIPM : public G4VSensitiveDetector
{
public:
  SensitiveSIPM(G4String name);
  ~SensitiveSIPM();
  
  // Main processing method - simplified to just count photons after surface processing
  G4bool ProcessHits(G4Step * step, G4TouchableHistory *);
  
  // Photon counting methods
  G4bool CountPhotonTouching(G4int trackID);
  G4bool CountPhotonIn(G4double wavelength, G4double angleDeg);
  G4bool CountPhotonDetected(G4double wavelength, G4double angleDeg);
  void ResetForFirstEvent(){fLastTrackID = -1;}

private:
  CF4EventAction * fEventAction;
  G4int fLastTrackID;
  G4double fhc;
};

#endif
