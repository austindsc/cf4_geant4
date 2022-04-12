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
class B1DetectorConstruction;
class G4SingleParticleSource;

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
    // const G4GeneralParticleSource* GetParticleSource() const { return fGunGamma1; }
  
  protected:
    // void build_Cs137(G4ThreeVector position, G4Tubs * tube);
    // void build_Co60(G4ThreeVector position, G4Tubs * tube);
    void doSourcePlacement(G4ThreeVector position, G4Tubs * tube, G4String name);
    void setGammaDirection(G4double thetaSpread, G4SingleParticleSource* gun);
  
  private:
    G4GeneralParticleSource* fGun;
    
    G4double fAlphaIntensity; //index 0
    G4double fGammaIntensity;  //index 1
    // G4double fGammaThetaMax;
    
    G4Tubs* fCF4_volume;
    G4VPhysicalVolume* famo2_volume;
    G4Tubs* famo2_tubs;
    
    G4VPhysicalVolume* fgammaSource_volume;
    G4Tubs* fgammaSource_tubs;
    G4VPhysicalVolume* fcollimator_volume;
    
    B1DetectorConstruction * fdetconstruction;
    
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
