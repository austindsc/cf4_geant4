/// \file B1PrimaryGeneratorAction.hh
/// \brief Definition of the B1PrimaryGeneratorAction class

#ifndef B1PrimaryGeneratorAction_h
#define B1PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "globals.hh"


class G4GeneralParticleSource;
class G4Event;
class G4Tubs;
class G4VPhysicalVolume;

/// The primary generator action class with particle gun.
///
/// The default kinematic is a 6 MeV gamma, randomly distribued 
/// in front of the phantom across 80% of the (X,Y) phantom size.

class B1PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    B1PrimaryGeneratorAction();    
    virtual ~B1PrimaryGeneratorAction();

    // method from the base class
    virtual void GeneratePrimaries(G4Event*);
  
    // method to access particle gun
    const G4GeneralParticleSource* GetParticleSource() const { return fGun; }
  
  private:
    G4GeneralParticleSource* fGun;
    G4Tubs* fCF4_volume;
    G4VPhysicalVolume* famo2_volume;
    G4Tubs* famo2_tubs;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
