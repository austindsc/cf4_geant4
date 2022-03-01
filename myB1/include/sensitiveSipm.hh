#ifndef SENSITIVESIPM_H
#define SENSITIVESIPM_H

#include "G4VSensitiveDetector.hh"
#include "B1EventAction.hh"

class sensitiveSipm : public G4VSensitiveDetector
{
public:
  sensitiveSipm(G4String);
  ~sensitiveSipm();  
  
private:
  virtual G4bool ProcessHits(G4Step *, G4TouchableHistory *);
  
  B1EventAction * fEventAction;
  
};

#endif