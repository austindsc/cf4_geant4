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

  // default particle kinematic is straight alpha
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  G4String particleName;
  G4ParticleDefinition* particle = particleTable->FindParticle(particleName="alpha");
  
  fGun->SetParticleDefinition(particle); 
  // 
  // fGun->SetParticleMomentumDirection(G4ThreeVector(0.,0.,-1.));
  // fGun->SetParticleEnergy(10.8 * MeV);
  
  // G4VPhysicalVolume * phys_alphaVolume = G4PhysicalVolumeStore::GetInstance()->GetVolume("phys_amo2");
  // G4LogicalVolume * logic_alphaVolume = phys_alphaVolume->GetLogicalVolume();
  // 
  // //   if ( cf4 ) fCF4_volume = dynamic_cast<G4Tubs*>(cf4->GetSolid());
  // 
  // // G4cout << "phys_amo2 has position " << phys_alphaVolume->GetObjectTranslation().getZ() << G4endl;
  // // exit(0);
  
  // G4double size = 0.8;
  G4double x0 = 0.;
  G4double y0 = 0.;
  G4double z0 = 10. * cm;
  
  // fGun->SetParticlePosition(G4ThreeVector(x0,y0,z0));
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
  //

  // In order to avoid dependence of PrimaryGeneratorAction
  // on DetectorConstruction class we get Envelope volume
  // from G4LogicalVolumeStore.
  
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
    
  }
  
  // got the source physical volume. Now we do our gun stuff
  if ( famo2_volume ) {
    
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
    
    tSource->GetEneDist()->SetMonoEnergy(5.39 * MeV);
  
    
    // fGun->GetCurrentSource()->GetEneDist()->SetEmax(10.0 * MeV);
    // G4cout << fGun->GetCurrentSource()->GetPosDist()->GetPosDisType() << G4endl;
    // exit(0);
    // // Get the pointer to the User Interface manager
    // G4UImanager* UImanager = G4UImanager::GetUIpointer();
    // UImanager->ApplyCommand("/gps/energy 5.4 MeV");
    // UImanager->ApplyCommand("/gps/pos/direction 0 0 -1");
   
    // // these defaults should remain, until simulating gammas
    // UImanager->ApplyCommand("/gps/pos/type Plane");
    // UImanager->ApplyCommand("/gps/pos/shape Circle");
    // UImanager->ApplyCommand("/gps/pos/centre 0 0 5 cm");
    // UImanager->ApplyCommand("/gps/pos/radius 2 mm");
    // UImanager->ApplyCommand("/gps/pos/confine phys_amo2");
  }  
  else  {
    G4ExceptionDescription msg;
    msg << "Cannot find amo2 volume\n"; 
    G4Exception("B1PrimaryGeneratorAction::GeneratePrimaries()", "MyCode0002",JustWarning,msg);

  }
  
  

  // G4double size = 0.8; 
  // G4double x0 = size * envSizeXY * (G4UniformRand()-0.5);
  // G4double y0 = size * envSizeXY * (G4UniformRand()-0.5);
  // G4double z0 = -0.5 * envSizeZ;
  

  // 
  fGun->GeneratePrimaryVertex(anEvent);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

