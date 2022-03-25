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

#include "G4RotationMatrix.hh"

#include "G4Tubs.hh"
#include "G4GeneralParticleSource.hh"
#include "G4UImanager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4SingleParticleSource.hh"

#include "G4RunManager.hh"
#include "B1DetectorConstruction.hh"
#include "myGlobals.hh"

#include <stdlib.h>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1PrimaryGeneratorAction::B1PrimaryGeneratorAction()
: G4VUserPrimaryGeneratorAction(), fCF4_volume(0), famo2_volume(0), famo2_tubs(0),
fgammaSource_volume(0), fgammaSource_tubs(0), fcollimator_volume(0), fdetconstruction(0)
{
  G4int n_particle = 1;
  fGunAlpha = new G4GeneralParticleSource();
  fGunAlpha->SetNumberOfParticles(n_particle);
  
  fGunGamma1 = new G4GeneralParticleSource();
  fGunGamma1->SetNumberOfParticles(n_particle);
  
  // // default particle kinematic is straight alpha
  // G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  // G4String particleName;
  // G4ParticleDefinition* particle = particleTable->FindParticle(particleName="alpha");
  // fGun->SetParticleDefinition(particle); 
  
  // G4cout << "runMan gives: " << detConst->GetGammaHeight()/cm << G4endl;
  // 
  // exit(0);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1PrimaryGeneratorAction::~B1PrimaryGeneratorAction()
{
  delete fGunAlpha;
  delete fGunGamma1;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B1PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  
  if(!fdetconstruction){
    fdetconstruction = (B1DetectorConstruction *)G4RunManager::GetRunManager()->GetUserDetectorConstruction();
    
    if(!fdetconstruction){
      G4ExceptionDescription msg;
      msg << "Can't find B1DetectorConstruction\n"; 
      G4Exception("B1PrimaryGeneratorAction::GeneratePrimaries()", 
      "Austin's warning message", FatalException, msg);
      }
  }
  
  // Setting up Alpha Source
  if (!famo2_volume && fdetconstruction->GetAlphaStatus() == true) {
    
    famo2_volume = G4PhysicalVolumeStore::GetInstance()->GetVolume("phys_amo2");
    famo2_tubs = dynamic_cast<G4Tubs*>(famo2_volume->GetLogicalVolume()->GetSolid());
    // G4LogicalVolume * logic_alphaVolume = phys_alphaVolume->GetLogicalVolume();
    // G4cout << "phys_amo2 has position " << famo2_volume->GetObjectTranslation().getZ()/mm << " mm" << G4endl;
    
    if(!famo2_tubs || !famo2_volume) {
      G4ExceptionDescription msg;
      msg << "Can't find famo2_volume (alpha source volume)\n"; 
      G4Exception("B1PrimaryGeneratorAction::GeneratePrimaries()",
      "Austin's warning message", FatalException, msg);
    }

    // default particle kinematic is alpha
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4String particleName;
    G4ParticleDefinition* particle = particleTable->FindParticle(particleName="alpha");
    fGunAlpha->SetParticleDefinition(particle); 
    
    // position distribution
    G4SingleParticleSource * tSource = fGunAlpha->GetCurrentSource();
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
    tSource->GetEneDist()->SetMonoEnergy(5.39 * MeV);
  }
  
  // Setting up Alpha Source
  if (!fgammaSource_volume && fdetconstruction->GetGammaStatus() == true){
    
    fgammaSource_volume = G4PhysicalVolumeStore::GetInstance()->GetVolume("phys_gammaSource");
    fgammaSource_tubs = dynamic_cast<G4Tubs*>(fgammaSource_volume->GetLogicalVolume()->GetSolid());
    fcollimator_volume = G4PhysicalVolumeStore::GetInstance()->GetVolume("phys_collimator");
  
    // need to rotate source vector
    const G4RotationMatrix * rot_coll = fcollimator_volume->GetRotation();
    G4ThreeVector collVec = fcollimator_volume->GetTranslation();
    G4ThreeVector ogsVec = fgammaSource_volume->GetTranslation();
    G4ThreeVector newGSVec = CLHEP::inverseOf(*rot_coll) * ogsVec;
    G4ThreeVector finalgsVector = newGSVec + collVec;
  
    // G4cout << "collimator vector: " << collVec/cm << " cm" << G4endl;
    // G4cout << "\nOriginal source vector: " << ogsVec/cm << G4endl;
    // G4cout << "new source vector: " << newGSVec/cm << " cm" << G4endl;
  
    if(!fgammaSource_volume || !fgammaSource_tubs || !fcollimator_volume) {
      G4ExceptionDescription msg;
      msg << "Can't find fgammaSource_tubs, _volume or fcollimator_volume \n"; 
      G4Exception("B1PrimaryGeneratorAction::GeneratePrimaries()",
      "Austin's warning message", FatalException, msg);
    } // all good, found source volume. Now set 'default' properties (do one time hopefully)
  
    // default particle kinematic is alpha
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4String particleName;
    G4ParticleDefinition* particle = particleTable->FindParticle(particleName="gamma");
    // G4ParticleDefinition* particle = particleTable->FindParticle(particleName="e-"); // for testing
    fGunGamma1->SetParticleDefinition(particle); 
  
    // position distribution
    G4SingleParticleSource * tSource = fGunGamma1->GetCurrentSource();
    tSource->GetPosDist()->SetPosDisType("Volume");
    tSource->GetPosDist()->SetPosDisShape("Cylinder");
    tSource->GetPosDist()->SetCentreCoords(finalgsVector);
    tSource->GetPosDist()->SetRadius(fgammaSource_tubs->GetOuterRadius());
    tSource->GetPosDist()->SetHalfZ(fgammaSource_tubs->GetZHalfLength());
    
    // tSource->GetPosDist()->SetCentreCoords(G4ThreeVector(0*cm, 0*cm, 3*cm)); // for testing
    // tSource->GetAngDist()->SetParticleMomentumDirection(G4ThreeVector(-1, 0, 0)); //for testing
    
    // angular distribution
    tSource->GetAngDist()->DefineAngRefAxes("angref1", G4ThreeVector(0, 0, -1));
    tSource->GetAngDist()->DefineAngRefAxes("angref2", G4ThreeVector(0, 1, 0));
    tSource->GetAngDist()->SetUseUserAngAxis(true);
    tSource->GetAngDist()->SetAngDistType("iso");
    tSource->GetAngDist()->SetMaxTheta(4. * deg);
  
    // energy distribution
    tSource->GetEneDist()->SetMonoEnergy(1. * MeV);
  }
  
  if(fdetconstruction->GetGammaStatus())fGunGamma1->GeneratePrimaryVertex(anEvent);
  if(fdetconstruction->GetAlphaStatus())fGunAlpha->GeneratePrimaryVertex(anEvent);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

