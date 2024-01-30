#ifndef SensitiveSIPM_H
#define SensitiveSIPM_H

#include "G4VSensitiveDetector.hh"
#include "G4PhysicsFreeVector.hh"

class G4UniformRand;
class CF4EventAction;

class SensitiveSIPM : public G4VSensitiveDetector
{
public:
  SensitiveSIPM(G4String);
  ~SensitiveSIPM();  
  G4PhysicsFreeVector * ovecPDE;
  
  G4bool ProcessHits(G4Step * step, G4TouchableHistory *);
  
  inline G4bool CountPhotonTouching(G4int trackID);
  inline G4bool CountPhotonIn(const G4Step * step);
private:
  CF4EventAction * fEventAction;
  G4int fLastTrackID; // use this to track photons from the same track (avoid double counting reflections)
  G4double fhc;

};

#endif
