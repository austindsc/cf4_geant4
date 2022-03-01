/// \file B1PrimaryGeneratorAction.cc
/// \brief Implementation of the B1PrimaryGeneratorAction class

#include "B1PrimaryGeneratorAction.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4RunManager.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include "G4Tubs.hh"
#include "G4GeneralParticleSource.hh"
#include "G4UImanager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4SingleParticleSource.hh"

#include <stdlib.h>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1PrimaryGeneratorAction::B1PrimaryGeneratorAction()
: G4VUserPrimaryGeneratorAction(),
  fCF4_volume(0), famo2_volume(0)
{
  G4int n_particle = 1;
  fGun = new G4GeneralParticleSource();
  fGun->SetNumberOfParticles(n_particle);

  // // default particle kinematic is straight alpha
  // G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  // G4String particleName;
  // G4ParticleDefinition* particle = particleTable->FindParticle(particleName="alpha");
  // fGun->SetParticleDefinition(particle); 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1PrimaryGeneratorAction::~B1PrimaryGeneratorAction()
{
  delete fGun;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B1PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  
  //this function is called at the begining of each event
  
  G4double cf4Top = 0;
  G4double cf4Bottom = 0;

  // first time it runs, we go here and apply all our 'default' behaviour
  if (!famo2_volume) {
    famo2_volume = G4PhysicalVolumeStore::GetInstance()->GetVolume("phys_amo2");
    
    // getting the solid
    famo2_tubs = dynamic_cast<G4Tubs*>(famo2_volume->GetLogicalVolume()->GetSolid());
    
    // G4LogicalVolume * logic_alphaVolume = phys_alphaVolume->GetLogicalVolume();
    // G4cout << "phys_amo2 has position " << famo2_volume->GetObjectTranslation().getZ()/mm << " mm" << G4endl;
    
    if(!famo2_tubs || !famo2_volume) {
      G4ExceptionDescription msg;
      msg << "Can't find famo2_tubs (source solid volume)\n"; 
      G4Exception("B1PrimaryGeneratorAction::GeneratePrimaries()",
      "Austin's warning message", FatalException, msg);
    }
    
    // all good, found source volume. Now set 'default' properties (do one time hopefully)
    
    // default particle kinematic is alpha
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4String particleName;
    G4ParticleDefinition* particle = particleTable->FindParticle(particleName="alpha");
    fGun->SetParticleDefinition(particle); 
    
    // position distribution
    G4SingleParticleSource * tSource = fGun->GetCurrentSource();
    tSource->GetPosDist()->SetPosDisType("Volume");
    tSource->GetPosDist()->SetPosDisShape("Cylinder");
    tSource->GetPosDist()->SetCentreCoords(famo2_volume->GetObjectTranslation());
    tSource->GetPosDist()->SetRadius(famo2_tubs->GetOuterRadius());
    tSource->GetPosDist()->SetHalfZ(famo2_tubs->GetZHalfLength());
    tSource->GetPosDist()->ConfineSourceToVolume("phys_amo2"); //redundant?
    
    // angular distribution
    tSource->GetAngDist()->SetAngDistType("iso");
    tSource->GetAngDist()->SetMaxTheta(90. * deg);
    
    // energy distribution
    // tSource->GetEneDist()->SetMonoEnergy(5.39 * MeV);
    tSource->GetEneDist()->SetMonoEnergy(5.39 * MeV);
  }
  
  // don't need to do anything, already done
  if ( famo2_volume ) {}

  fGun->GeneratePrimaryVertex(anEvent);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

