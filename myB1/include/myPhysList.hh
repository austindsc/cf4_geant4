#ifndef myPhysList_h
#define myPhysList_h

#include "G4VModularPhysicsList.hh"

class myPhysList : public G4VModularPhysicsList{
public:
  myPhysList();
  
  virtual void ConstructParticle();
  virtual void ConstructProcess();
  virtual void SetCuts();
  ~myPhysList();
  
};

#endif