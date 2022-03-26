#ifndef SENSITIVESIPM_H
#define SENSITIVESIPM_H

#include "G4VSensitiveDetector.hh"
#include "B1EventAction.hh"
#include "G4PhysicsOrderedFreeVector.hh"

class G4UniformRand;

class sensitiveSipm : public G4VSensitiveDetector
{
public:
  sensitiveSipm(G4String);
  ~sensitiveSipm();  
  
private:
  B1EventAction * fEventAction;

  virtual G4bool ProcessHits(G4Step *, G4TouchableHistory *);
  
  G4PhysicsOrderedFreeVector * ovecPDE;

};

#endif