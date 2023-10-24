/// \file CF4PrimaryGeneratorAction.cc
/// \brief Implementation of the CF4PrimaryGeneratorAction class

#include "CF4PrimaryGeneratorAction.hh"

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
#include "CF4DetectorConstruction.hh"
#include "myGlobals.hh"

#include "Randomize.hh"
#include "G4SPSRandomGenerator.hh"

#include "G4IonTable.hh"

#include <stdlib.h>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CF4PrimaryGeneratorAction::CF4PrimaryGeneratorAction()
: G4VUserPrimaryGeneratorAction(), fCF4_volume(0), famo2_volume(0), famo2_tubs(0),
fgammaSource_volume(0), fgammaSource_tubs(0), fcollimator_volume(0), fdetconstruction(0)
{
  G4int n_particle = 1;
  fGun = new G4GeneralParticleSource();
  fGun->SetNumberOfParticles(n_particle);
  
  fAlphaIntensity = 10;
  fGammaIntensity = 10;
  // fGammaThetaMax = 1.5*deg;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CF4PrimaryGeneratorAction::~CF4PrimaryGeneratorAction()
{
  delete fGun;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CF4PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  
  if(!fdetconstruction){
    fdetconstruction = (CF4DetectorConstruction *)G4RunManager::GetRunManager()->GetUserDetectorConstruction();
    if(!fdetconstruction){
      G4ExceptionDescription msg;
      msg << "Can't find CF4DetectorConstruction\n"; 
      G4Exception("CF4PrimaryGeneratorAction::GeneratePrimaries()", "Austin's warning message", FatalException, msg);
    }
  }
  
  // Setting up Alpha Source
  if (!famo2_volume && fdetconstruction->GetAlphaStatus() == true) {
    
    fGun->SetCurrentSourceIntensity(fAlphaIntensity);
    
    famo2_volume = G4PhysicalVolumeStore::GetInstance()->GetVolume("phys_amo2");
    famo2_tubs = dynamic_cast<G4Tubs*>(famo2_volume->GetLogicalVolume()->GetSolid());
    // G4LogicalVolume * logic_alphaVolume = phys_alphaVolume->GetLogicalVolume();
    // G4cout << "phys_amo2 has position " << famo2_volume->GetObjectTranslation().getZ()/mm << " mm" << G4endl;
    G4VPhysicalVolume * phys_medium = G4PhysicalVolumeStore::GetInstance()->GetVolume("phys_detectionMedium");
    
    if(!famo2_tubs || !famo2_volume) {
      G4ExceptionDescription msg;
      msg << "Can't find famo2_volume (alpha source volume)\n"; 
      G4Exception("CF4PrimaryGeneratorAction::GeneratePrimaries()",
      "Austin's warning message", FatalException, msg);
    }

    G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle("alpha");
    fGun->SetParticleDefinition(particle); 
    G4ThreeVector alphaSvector = famo2_volume->GetObjectTranslation() + phys_medium->GetObjectTranslation();
    this->doSourcePlacement(alphaSvector, famo2_tubs, "phys_amo2");
    // this->doSourcePlacement(G4ThreeVector(0, 0, 2*cm), famo2_tubs, "phys_amo2"); // testing
    
    // angular distribution
    fGun->GetCurrentSource()->GetAngDist()->SetAngDistType("iso");
    fGun->GetCurrentSource()->GetAngDist()->SetMaxTheta(90. * deg);
    
    // energy distribution
    fGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(5.39 * MeV);
  }
  
  // Setting up Gamma Source
  if (!fgammaSource_volume && fdetconstruction->GetGammaStatus() == true){
    
    // kill the alpha source if its not requested
    if(fdetconstruction->GetAlphaStatus() == false){
      fGun->SetCurrentSourceto(0); // alpha source
      fGun->SetCurrentSourceIntensity(0);
    }
    
    fgammaSource_volume = G4PhysicalVolumeStore::GetInstance()->GetVolume("phys_gammaSource");
    fgammaSource_tubs = dynamic_cast<G4Tubs*>(fgammaSource_volume->GetLogicalVolume()->GetSolid());
    
    G4ThreeVector collVec = G4ThreeVector(0, 0, 0);
    const G4RotationMatrix * rot_coll = new G4RotationMatrix();
    if(fdetconstruction->GetCollStatus() == true){ //using collimator
      fcollimator_volume = G4PhysicalVolumeStore::GetInstance()->GetVolume("phys_collimator");
  
      // need to rotate source vector
      rot_coll = fcollimator_volume->GetRotation();
      collVec = fcollimator_volume->GetTranslation();
    }
    
    G4ThreeVector ogsVec = fgammaSource_volume->GetTranslation();
    G4ThreeVector newGSVec = CLHEP::inverseOf(*rot_coll) * ogsVec;
    G4ThreeVector finalgsVector = newGSVec + collVec;
    // finalgsVector = G4ThreeVector(0, 0 , 4*cm); // testing
  
    // G4cout << "collimator vector: " << collVec/cm << " cm" << G4endl;
    // G4cout << "\nOriginal source vector: " << ogsVec/cm << G4endl;
    // G4cout << "new source vector: " << newGSVec/cm << " cm" << G4endl;
  
    G4double userThetaMax = fdetconstruction->GetGammaThetaMax();
    if((!fgammaSource_volume || !fgammaSource_tubs) || (!fcollimator_volume && fdetconstruction->GetCollStatus()==true)) {
      G4ExceptionDescription msg;
      msg << "Can't find fgammaSource_tubs, _volume or fcollimator_volume \n"; 
      G4Exception("CF4PrimaryGeneratorAction::GeneratePrimaries()",
      "Austin's warning message", FatalException, msg);
    } // all good, found source volume. Now set 'default' properties (do one time hopefully)
  
    G4String gs = fdetconstruction->GetGammaSourceStr();
    if(gs == "isoCs137"){
      fGun->AddaSource(fGammaIntensity);
      this->doSourcePlacement(finalgsVector, fgammaSource_tubs, "phys_gammaSource");
      fGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(0.0*MeV);
      fGun->GetCurrentSource()->SetParticleDefinition(G4IonTable::GetIonTable()->GetIon(55, 137, 0.*keV));
      fGun->GetCurrentSource()->SetParticleCharge(0*eplus);
    }
    else if(gs == "isoCo60"){
      fGun->AddaSource(fGammaIntensity);
      this->doSourcePlacement(finalgsVector, fgammaSource_tubs, "phys_gammaSource");
      fGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(0.0*MeV);
      fGun->GetCurrentSource()->SetParticleDefinition(G4IonTable::GetIonTable()->GetIon(27, 60, 0.*keV));
      fGun->GetCurrentSource()->SetParticleCharge(0*eplus);
    }
    else if(gs == "isoCo57"){
      fGun->AddaSource(fGammaIntensity);
      this->doSourcePlacement(finalgsVector, fgammaSource_tubs, "phys_gammaSource");
      fGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(0.0*MeV);
      fGun->GetCurrentSource()->SetParticleDefinition(G4IonTable::GetIonTable()->GetIon(27, 57, 0.*keV));
      fGun->GetCurrentSource()->SetParticleCharge(0*eplus);
      
    }
    else if(gs == "Cs137"){
      const int csN = 8;
      // taken from https://www.nndc.bnl.gov/nudat3/decaysearchdirect.jsp?nuc=137Cs&unc=NDS
      G4double myCsEnergy[csN] = {4.47*keV, 31.817*keV, 32.194*keV, 36.304*keV, 36.378*keV, 37.255*keV, 283.5*keV, 0.661*MeV};
      G4double myCsIntensity[csN] = {0.0091, 0.0199, 0.0364, 0.00348, 0.00672, 0.00213, 0.0000058, 0.851};
      
      for(int i = 0; i < csN; i++){
        fGun->AddaSource(fGammaIntensity * myCsIntensity[i]);
        fGun->GetCurrentSource()->SetParticleDefinition(G4ParticleTable::GetParticleTable()->FindParticle("gamma"));
        this->doSourcePlacement(finalgsVector, fgammaSource_tubs, "phys_gammaSource");
        // this->doSourcePlacement(G4ThreeVector(0, 0, 4), fgammaSource_tubs, "phys_gammaSource"); // testing
        this->setGammaDirection(userThetaMax, fGun->GetCurrentSource());
        fGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(myCsEnergy[i]);
      }
      
    }
    else if(gs == "Co60"){
      const int coN = 8;
      // taken from https://www.nndc.bnl.gov/nudat3/decaysearchdirect.jsp?nuc=60Co&unc=NDS
      G4double myCoEnergy[coN] = {7.47*keV, 8.265*keV, 347.14*keV, 826.1*keV, 1173.228*keV, 1332.5*keV, 2158.6*keV, 2505.7*keV};
      G4double myCoIntensity[coN] = {0.000063+0.000032, 0.000015, 0.000075, 0.000076, 0.9985, 0.9998, 0.000012, 2e-8};
      
      for(int i = 0; i < coN; i++){
        fGun->AddaSource(fGammaIntensity * myCoIntensity[i]);
        fGun->GetCurrentSource()->SetParticleDefinition(G4ParticleTable::GetParticleTable()->FindParticle("gamma"));
        this->doSourcePlacement(finalgsVector, fgammaSource_tubs, "phys_gammaSource");
        // this->doSourcePlacement(G4ThreeVector(0, 0, 4), fgammaSource_tubs, "phys_gammaSource"); // testing
        this->setGammaDirection(userThetaMax, fGun->GetCurrentSource());
        fGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(myCoEnergy[i]);
      }
    }
    else if(gs == "Co57"){
      const int coN = 12;
      // taken from https://www.nndc.bnl.gov/nudat3/decaysearchdirect.jsp?nuc=57Co&unc=NDS
      G4double myCoEnergy[coN] = {6.4*keV, 7.058*keV, 14.4*keV, 122.06*keV, 136.47*keV, 230.4*keV, 339.7*keV, 352.33*keV, 366.8*keV, 570.*keV, 692.4*keV, 706.54*keV};
      G4double myCoIntensity[coN] = { (0.166 + 0.33), 0.059, 0.091, 0.856, 0.107, 4.0e-6, 3.7e-5, 3.e-5, 1.2e-5, 1.58e-4, 0.00149, 5.0e-5};
      
      for(int i = 0; i < coN; i++){
        fGun->AddaSource(fGammaIntensity * myCoIntensity[i]);
        fGun->GetCurrentSource()->SetParticleDefinition(G4ParticleTable::GetParticleTable()->FindParticle("gamma"));
        this->doSourcePlacement(finalgsVector, fgammaSource_tubs, "phys_gammaSource");
        // this->doSourcePlacement(G4ThreeVector(0, 0, 4), fgammaSource_tubs, "phys_gammaSource"); // testing
        this->setGammaDirection(userThetaMax, fGun->GetCurrentSource());
        fGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(myCoEnergy[i]);
      }
    }
  }
  
  fGun->GeneratePrimaryVertex(anEvent);
}

void CF4PrimaryGeneratorAction::doSourcePlacement(G4ThreeVector position, G4Tubs * tube, G4String name){
  G4SingleParticleSource * tSource = fGun->GetCurrentSource();
  tSource->GetPosDist()->SetPosDisType("Volume");
  tSource->GetPosDist()->SetPosDisShape("Cylinder");
  tSource->GetPosDist()->SetCentreCoords(position);
  tSource->GetPosDist()->SetRadius(tube->GetOuterRadius());
  tSource->GetPosDist()->SetHalfZ(tube->GetZHalfLength());
  tSource->GetPosDist()->ConfineSourceToVolume(name);
  return;
}

void CF4PrimaryGeneratorAction::setGammaDirection(G4double thetaSpread, G4SingleParticleSource* gun){
  // angular distribution
  gun->GetAngDist()->DefineAngRefAxes("angref1", G4ThreeVector(0, 0, -1));
  gun->GetAngDist()->DefineAngRefAxes("angref2", G4ThreeVector(0, 1, 0));
  gun->GetAngDist()->SetUseUserAngAxis(true);
  gun->GetAngDist()->SetAngDistType("iso");
  gun->GetAngDist()->SetMaxTheta(thetaSpread);
  return;
}

// void CF4PrimaryGeneratorAction::build_Cs137(G4ThreeVector position, G4Tubs * tube);
// void CF4PrimaryGeneratorAction::build_Co60(G4ThreeVector position, G4Tubs * tube);

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

