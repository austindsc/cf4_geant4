#include "myPhysList.hh"

#include "G4OpticalPhysics.hh"
// #include "G4EmStandardPhysics_option4.hh"
#include "G4EmStandardPhysics.hh"

#include "G4Scintillation.hh"

myPhysList::myPhysList(){
  
  G4EmStandardPhysics* emPhys = new G4EmStandardPhysics();
  G4OpticalPhysics* opticalPhysics = new G4OpticalPhysics();
  
  opticalPhysics->Configure(kCerenkov, false);
  opticalPhysics->Configure(kScintillation, true);
  opticalPhysics->SetCerenkovTrackSecondariesFirst(true);
  opticalPhysics->SetTrackSecondariesFirst(kScintillation, true);
  
  // G4Scintillation* theScintProcessDef = new G4Scintillation("Scintillation");
  // // theScintProcessDef->DumpPhysicsTable();
  // theScintProcessDef->SetTrackSecondariesFirst(true);
  // theScintProcessDef->SetScintillationYieldFactor(10.0); //
  // theScintProcessDef->SetScintillationExcitationRatio(1.0); //
  // theScintProcessDef->SetVerboseLevel(1);
  
  
  RegisterPhysics(emPhys);
  RegisterPhysics(opticalPhysics);
}

myPhysList::~myPhysList(){}