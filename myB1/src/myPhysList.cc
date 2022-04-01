#include "myPhysList.hh"

#include "G4OpticalPhysics.hh"
#include "G4OpticalParameters.hh"
// #include "G4EmStandardPhysics_option4.hh"
#include "G4EmStandardPhysics.hh"

#include "G4Scintillation.hh"

#include "G4StepLimiterPhysics.hh"

myPhysList::myPhysList(){
  
  G4EmStandardPhysics* emPhys = new G4EmStandardPhysics();
  G4OpticalPhysics* opticalPhysics = new G4OpticalPhysics();
  
  G4OpticalParameters * oppars = G4OpticalParameters::Instance();
  
  oppars->SetCerenkovTrackSecondariesFirst(true);
  oppars->SetScintTrackSecondariesFirst(true);
  oppars->SetScintFiniteRiseTime(true);
  oppars->SetScintByParticleType(false);
    
  // opticalParams->SetCerenkovTrackSecondariesFirst(true);
  // opticalParams->SetScintTrackSecondariesFirst(true);
  
  // G4Scintillation* theScintProcessDef = new G4Scintillation("Scintillation");
  // // theScintProcessDef->DumpPhysicsTable();
  // theScintProcessDef->SetTrackSecondariesFirst(true);
  // theScintProcessDef->SetScintillationYieldFactor(10.0); //
  // theScintProcessDef->SetScintillationExcitationRatio(1.0); //
  // theScintProcessDef->SetVerboseLevel(1);
  
  G4StepLimiterPhysics * myLimiter = new G4StepLimiterPhysics();
  myLimiter->SetApplyToAll(true);
  
  // myLimiter->ConstructParticle("opticalphoton");
  RegisterPhysics(myLimiter);
  
  RegisterPhysics(emPhys);
  RegisterPhysics(opticalPhysics);
}

myPhysList::~myPhysList(){}