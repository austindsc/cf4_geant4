#include "myPhysList.hh"

#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include "G4EmStandardPhysics.hh"
#include "G4EmParameters.hh"

#include "G4OpticalPhysics.hh"
#include "G4OpticalParameters.hh"
#include "G4Scintillation.hh"

#include "G4ParticleTypes.hh"
#include "G4IonConstructor.hh"
#include "G4PhysicsListHelper.hh"
#include "G4Radioactivation.hh"
#include "G4NuclideTable.hh"
#include "G4LossTableManager.hh"
#include "G4UAtomicDeexcitation.hh"
#include "G4NuclearLevelData.hh"
#include "G4DeexPrecoParameters.hh"
#include "G4NuclideTable.hh"

// #include "G4EmExtraPhysics.hh"
#include "G4StepLimiterPhysics.hh"

myPhysList::myPhysList() : G4VModularPhysicsList(){
  
  // EM physics
  G4EmStandardPhysics* emPhys = new G4EmStandardPhysics();
  RegisterPhysics(emPhys);
  G4EmParameters * emPar = G4EmParameters::Instance();
  emPar->SetAugerCascade(true);

  // optical physics
  G4OpticalPhysics* opticalPhysics = new G4OpticalPhysics();
  G4OpticalParameters * oppars = G4OpticalParameters::Instance();
  oppars->SetCerenkovTrackSecondariesFirst(true);
  oppars->SetScintTrackSecondariesFirst(true);
  oppars->SetScintFiniteRiseTime(true);
  oppars->SetScintByParticleType(false);

  RegisterPhysics(opticalPhysics);
  
  // user limits (sketchy)
  G4StepLimiterPhysics * myLimiter = new G4StepLimiterPhysics();
  myLimiter->SetApplyToAll(true);
  // myLimiter->ConstructParticle("opticalphoton");
  RegisterPhysics(myLimiter);
  
  
  // Mandatory for G4NuclideTable
  // Half-life threshold must be set small or many short-lived isomers 
  // will not be assigned life times (default to 0) 
  G4NuclideTable::GetInstance()->SetThresholdOfHalfLife(0.1*picosecond);
  G4NuclideTable::GetInstance()->SetLevelTolerance(1.0*eV);
  
  // Gamma-Nuclear Physics
  G4DeexPrecoParameters* deex = G4NuclearLevelData::GetInstance()->GetParameters();
  deex->SetCorrelatedGamma(true);
  deex->SetStoreAllLevels(true);
  deex->SetIsomerProduction(true);  
  deex->SetMaxLifeTime(G4NuclideTable::GetInstance()->GetThresholdOfHalfLife()
                /std::log(2.));
}

myPhysList::~myPhysList(){}


void myPhysList::ConstructProcess(){
  AddTransportation();
  
  G4Radioactivation* radioactiveDecay = new G4Radioactivation();

  G4bool ARMflag = false;
  radioactiveDecay->SetARM(ARMflag);        //Atomic Rearangement

  // need to initialize atomic deexcitation
  //
  G4LossTableManager* man = G4LossTableManager::Instance();
  G4VAtomDeexcitation* deex = man->AtomDeexcitation();
  if (!deex) {
     ///G4EmParameters::Instance()->SetFluo(true);
     G4EmParameters::Instance()->SetAugerCascade(ARMflag);
     G4EmParameters::Instance()->SetDeexcitationIgnoreCut(ARMflag);    
     deex = new G4UAtomicDeexcitation();
     deex->InitialiseAtomicDeexcitation();
     man->SetAtomDeexcitation(deex);
  }

  // register radioactiveDecay
  //
  G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();
  ph->RegisterProcess(radioactiveDecay, G4GenericIon::GenericIon());

  //printout
  //
  G4cout << "\n  Set atomic relaxation mode " << ARMflag << G4endl;
}

void myPhysList::ConstructParticle()
{
  // pseudo-particles
  G4Geantino::GeantinoDefinition();

  // gamma
  G4Gamma::GammaDefinition();

  // leptons
  G4Electron::ElectronDefinition();
  G4Positron::PositronDefinition();

  G4NeutrinoE::NeutrinoEDefinition();
  G4AntiNeutrinoE::AntiNeutrinoEDefinition();

  // baryons
  G4Proton::ProtonDefinition();
  G4Neutron::NeutronDefinition();  

  // ions
  G4IonConstructor iConstructor;
  iConstructor.ConstructParticle();  
}

void myPhysList::SetCuts(){
  SetCutsWithDefault();
  // SetCutValue(0*mm, "proton");
  // SetCutValue(10*km, "e-");
  // SetCutValue(10*km, "e+");
  // SetCutValue(10*km, "gamma");
}