//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "PhysicsList.hh"
// #include "PhysicsListMessenger.hh"

// #include "PhysListEmStandard.hh"

#include "G4EmStandardPhysics.hh"
#include "G4EmStandardPhysics_option1.hh"
#include "G4EmStandardPhysics_option2.hh"
#include "G4EmStandardPhysics_option3.hh"

#include "G4LossTableManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

// Em physics list
#include "G4BuilderType.hh"
#include "G4ParticleDefinition.hh"
#include "G4PhysicsListHelper.hh"

#include "G4ComptonScattering.hh"
#include "G4GammaConversion.hh"
#include "G4PhotoElectricEffect.hh"

#include "G4eMultipleScattering.hh"
#include "G4hMultipleScattering.hh"

#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"
#include "G4eplusAnnihilation.hh"

#include "G4MuIonisation.hh"
#include "G4MuBremsstrahlung.hh"
#include "G4MuPairProduction.hh"

#include "G4hIonisation.hh"
#include "G4hBremsstrahlung.hh"
#include "G4hPairProduction.hh"

#include "G4ionIonisation.hh"

#include "G4MscStepLimitType.hh"

#include "G4SystemOfUnits.hh"

// others from DMX list
//em process options to allow msc step-limitation to be switched off
#include "G4ProcessManager.hh"
#include "G4EmParameters.hh"
#include "G4VAtomDeexcitation.hh"
#include "G4UAtomicDeexcitation.hh"
#include "G4LossTableManager.hh"

// gamma
#include "G4PhotoElectricEffect.hh"
#include "G4LivermorePhotoElectricModel.hh"

#include "G4ComptonScattering.hh"
#include "G4LivermoreComptonModel.hh"

#include "G4GammaConversion.hh"
#include "G4BetheHeitler5DModel.hh"

#include "G4RayleighScattering.hh" 
#include "G4LivermoreRayleighModel.hh"

// e-
#include "G4eMultipleScattering.hh"

#include "G4eIonisation.hh"
#include "G4LivermoreIonisationModel.hh"

#include "G4eBremsstrahlung.hh"
#include "G4UniversalFluctuation.hh"

#include "G4Decay.hh"
#include "G4RadioactiveDecay.hh"
#include "G4PhysicsListHelper.hh"
#include "G4NuclideTable.hh"
#include "G4NuclearLevelData.hh"

// opticalPhysics
#include "G4Scintillation.hh"
#include "G4OpAbsorption.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4OpticalParameters.hh"

// blah
#include "G4RadioactiveDecay.hh"
#include "G4Radioactivation.hh"
#include "G4GenericIon.hh"

// particle makers
#include "G4BosonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4BosonConstructor.hh"
#include "G4BaryonConstructor.hh"
#include "G4IonConstructor.hh"
#include "G4ShortLivedConstructor.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PhysicsList::PhysicsList() : G4VUserPhysicsList()
{
  // pMessenger = new PhysicsListMessenger(this);
  SetVerboseLevel(1);

  // EM physics
  // emName = G4String("local");
  // emPhysicsList = new PhysListEmStandard(emName);
  
  SetDefaultCutValue(1*mm);
  G4LossTableManager::Instance();
  
  defaultCutValue     = 1.0*micrometer; //
  cutForGamma         = defaultCutValue;
  cutForElectron      = 1.0*nanometer;
  cutForPositron      = defaultCutValue;
  
  //set a finer grid of the physic tables in order to improve precision
  //former LowEnergy models have 200 bins up to 100 GeV
  G4EmParameters* param = G4EmParameters::Instance();
  param->SetMaxEnergy(10*GeV);
  param->SetNumberOfBinsPerDecade(20);
  param->SetMscStepLimitType(fMinimal);
  param->SetFluo(true);
  param->SetPixe(true);
  param->SetAuger(true);
  
  G4NuclideTable::GetInstance()->SetThresholdOfHalfLife(0.1*picosecond);
  G4NuclideTable::GetInstance()->SetLevelTolerance(1.0*eV);
  // G4EmParameters::Instance()->AddPhysics("World","G4RadioactiveDecay");
  G4DeexPrecoParameters* deex = G4NuclearLevelData::GetInstance()->GetParameters();
  deex->SetCorrelatedGamma(true);
  deex->SetStoreICLevelData(true);
  deex->SetIsomerProduction(true);  
  deex->SetMaxLifeTime(G4NuclideTable::GetInstance()->GetThresholdOfHalfLife()
                       /std::log(2.));
}

void PhysicsList::SetCuts(){
  //special for low energy physics
  G4double lowlimit=250*eV;  
  G4ProductionCutsTable::GetProductionCutsTable()->SetEnergyRange(lowlimit,100.*GeV);

  // set cut values for gamma at first and for e- second and next for e+,
  // because some processes for e+/e- need cut values for gamma 
  SetCutValue(cutForGamma, "gamma");
  SetCutValue(cutForElectron, "e-");
  SetCutValue(cutForPositron, "e+");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PhysicsList::~PhysicsList()
{
  // delete pMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::ConstructParticle()
{
    G4BosonConstructor  pBosonConstructor;
    pBosonConstructor.ConstructParticle();

    G4LeptonConstructor pLeptonConstructor;
    pLeptonConstructor.ConstructParticle();

    G4MesonConstructor pMesonConstructor;
    pMesonConstructor.ConstructParticle();

    G4BaryonConstructor pBaryonConstructor;
    pBaryonConstructor.ConstructParticle();

    G4IonConstructor pIonConstructor;
    pIonConstructor.ConstructParticle();

    G4ShortLivedConstructor pShortLivedConstructor;
    pShortLivedConstructor.ConstructParticle();  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "G4PhysicsListHelper.hh"
#include "G4Decay.hh"

void PhysicsList::ConstructProcess()
{
  AddTransportation();

  // electromagnetic Physics List
  //
  // emPhysicsList->ConstructProcess();
  
  ConstructEM();
  ConstructOptical();
  ConstructGeneral();

}

void PhysicsList::ConstructGeneral(){
  
  // Add Decay Process
  G4Radioactivation* radioactiveDecay = new G4Radioactivation();
  
  G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();

  // need to initialize atomic deexcitation
  G4LossTableManager* man = G4LossTableManager::Instance();
  G4VAtomDeexcitation* deex = man->AtomDeexcitation();
  if (!deex) {
     ///G4EmParameters::Instance()->SetFluo(true); 
     deex = new G4UAtomicDeexcitation();
     deex->InitialiseAtomicDeexcitation();
     man->SetAtomDeexcitation(deex);
  }
  
  ph->RegisterProcess(radioactiveDecay, G4GenericIon::GenericIon());
  // auto particleIterator=GetParticleIterator();
  // particleIterator->reset();
  // while( (*particleIterator)() ){
  //   G4ParticleDefinition* particle = particleIterator->value();
  //   if (fDecayProcess->IsApplicable(*particle)) 
  //     ph ->RegisterProcess(fDecayProcess, particle);
  // }
  
}

void PhysicsList::ConstructOptical(){
  G4OpticalParameters* opParams = G4OpticalParameters::Instance();
  
  G4Scintillation* theScintProcessDef = new G4Scintillation("Scintillation");
  opParams->SetScintTrackSecondariesFirst(true);
  // opParams->SetScintByParticleType(true);
  // // optical physics
  // G4OpticalPhysics* opticalPhysics = new G4OpticalPhysics();
  opParams->SetCerenkovTrackSecondariesFirst(true);
  opParams->SetScintTrackSecondariesFirst(true);
  opParams->SetScintFiniteRiseTime(true);
  opParams->SetScintByParticleType(false);

  // optical processes
  G4OpAbsorption* theAbsorptionProcess = new G4OpAbsorption();
  G4OpBoundaryProcess* theBoundaryProcess = new G4OpBoundaryProcess();

  auto particleIterator=GetParticleIterator();
  particleIterator->reset();
  while( (*particleIterator)() )
    {
      G4ParticleDefinition* particle = particleIterator->value();
      G4ProcessManager* pmanager = particle->GetProcessManager();
      G4String particleName = particle->GetParticleName();
      if (theScintProcessDef->IsApplicable(*particle)){
        pmanager->AddProcess(theScintProcessDef);
        pmanager->SetProcessOrderingToLast(theScintProcessDef,idxAtRest);
        pmanager->SetProcessOrderingToLast(theScintProcessDef,idxPostStep);
      }
      
      if (particleName == "opticalphoton"){
	       pmanager->AddDiscreteProcess(theAbsorptionProcess);
	       pmanager->AddDiscreteProcess(theBoundaryProcess);
      }
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::ConstructEM(){
  
  G4LossTableManager* man = G4LossTableManager::Instance();
  man->SetAtomDeexcitation(new G4UAtomicDeexcitation());
  
  G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();
  
  // Add DMX EM Processes
  auto particleIterator=GetParticleIterator();
  particleIterator->reset();
  while( (*particleIterator)() ){
    G4ParticleDefinition* particle = particleIterator->value();
    // G4ProcessManager* pmanager = particle->GetProcessManager();
    G4String particleName = particle->GetParticleName();
     
    if (particleName == "gamma") {
      // gamma         
      // pmanager->AddDiscreteProcess(new G4RayleighScattering());
      
      G4PhotoElectricEffect* thePhotoElectricEffect = new G4PhotoElectricEffect();
      thePhotoElectricEffect->SetEmModel(new G4LivermorePhotoElectricModel());
      // pmanager->AddDiscreteProcess(thePhotoElectricEffect);
      
      G4ComptonScattering* theComptonScattering = new G4ComptonScattering();
      theComptonScattering->SetEmModel(new G4LivermoreComptonModel());
      // pmanager->AddDiscreteProcess(theComptonScattering);
      
      G4GammaConversion* theGammaConversion = new G4GammaConversion();
      theGammaConversion->SetEmModel(new G4BetheHeitler5DModel());
      // pmanager->AddDiscreteProcess(theGammaConversion);
      
      ph->RegisterProcess(new G4RayleighScattering(), particle);
      ph->RegisterProcess(thePhotoElectricEffect, particle);
      ph->RegisterProcess(theComptonScattering,   particle);
      ph->RegisterProcess(theGammaConversion,     particle);
      
    } else if (particleName == "e-") {
      //electron
      // Multiple scattering
      G4eMultipleScattering* msc = new G4eMultipleScattering();
      msc->SetStepLimitType(fUseDistanceToBoundary);
      ph->RegisterProcess(msc, particle);
      
      // Ionisation
    	G4eIonisation* eIonisation = new G4eIonisation();
            G4VEmModel* theIoniLiv = new G4LivermoreIonisationModel();
            theIoniLiv->SetHighEnergyLimit(0.1*MeV); 
            eIonisation->AddEmModel(0, theIoniLiv, new G4UniversalFluctuation() );
    	eIonisation->SetStepFunction(0.2, 100*um); //improved precision in tracking 
      ph->RegisterProcess(eIonisation,         particle);
      ph->RegisterProcess(new G4eBremsstrahlung(),   particle);
      
    } else if (particleName == "e+") {
      //positron
      G4eMultipleScattering* msc = new G4eMultipleScattering();
      msc->SetStepLimitType(fUseDistanceToBoundary);
      ph->RegisterProcess(msc, particle);
      
      // Ionisation
      G4eIonisation* eIonisation = new G4eIonisation();
      eIonisation->SetStepFunction(0.2, 100*um);
      ph->RegisterProcess(eIonisation,         particle);
      ph->RegisterProcess(new G4eBremsstrahlung(),   particle);
      ph->RegisterProcess(new G4eplusAnnihilation,   particle);
      
    } else if( particleName == "mu-" || 
               particleName == "mu+"    ) {
      //muon  
      ph->RegisterProcess(new G4hMultipleScattering, particle);
      ph->RegisterProcess(new G4MuIonisation,        particle);
      ph->RegisterProcess(new G4MuBremsstrahlung,    particle);
      ph->RegisterProcess(new G4MuPairProduction,    particle);
             
    } else if( particleName == "proton" ||
               particleName == "pi-" ||
               particleName == "pi+"    ) {
      //proton  
      ph->RegisterProcess(new G4hMultipleScattering, particle);
      ph->RegisterProcess(new G4hIonisation,         particle);
      ph->RegisterProcess(new G4hBremsstrahlung,     particle);
      ph->RegisterProcess(new G4hPairProduction,     particle);       
     
    } else if( particleName == "alpha" || 
	       particleName == "He3" || 
	       particleName == "GenericIon" ) {
      //Ions 
      ph->RegisterProcess(new G4hMultipleScattering, particle);
      ph->RegisterProcess(new G4ionIonisation,       particle);

    } else if ((!particle->IsShortLived()) &&
	       (particle->GetPDGCharge() != 0.0) && 
	       (particle->GetParticleName() != "chargedgeantino")) {
      //all others charged particles except geantino
      ph->RegisterProcess(new G4hMultipleScattering, particle);
      ph->RegisterProcess(new G4hIonisation,         particle);
    }
  }
}
