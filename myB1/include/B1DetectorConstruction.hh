/// \file B1DetectorConstruction.hh
/// \brief Definition of the B1DetectorConstruction class

#ifndef B1DetectorConstruction_h
#define B1DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;
class B1DetectorConstructionMessenger;
class DetectorMaterials;

/// Detector construction class to define materials and geometry.
class B1DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    B1DetectorConstruction();
    virtual ~B1DetectorConstruction();

    virtual G4VPhysicalVolume* Construct();
    
    // getters and setters
    G4LogicalVolume* GetScoringVolume() const { return fScoringVolume; }
    
    G4double GetAlphaHeight() { return fAlphaHeight; }
    void SetAlphaHeight(G4double h) {
      this->SetAlphaStatus(true);
      fAlphaHeight = h;}
    G4bool GetAlphaStatus(){ return fAlphaStatus; }
    void SetAlphaStatus(G4bool y){ fAlphaStatus = y; }
    
    G4double GetGammaHeight() {return fGammaHeight; }
    void SetGammaHeight(G4double h){
      this->SetGammaStatus(true);
      fGammaHeight = h;
    }
    G4bool GetGammaStatus(){ return fGammaStatus; }
    void SetGammaStatus(G4bool y){ fGammaStatus = y; }
    
    G4bool GetCollStatus(){ return fCollStatus; }
    void SetCollStatus(G4bool y){ fCollStatus = y; }
    
    G4String GetGammaSourceStr(){ return fGammaSource; }
    void SetGammaSourceStr(G4String sourceName){ fGammaSource = sourceName; }

  protected:    
    G4LogicalVolume*  fScoringVolume; //(this is the cf4)
    G4LogicalVolume*  fsipmVolume;
    
    G4LogicalVolume*  fpcb;
    G4LogicalVolume*  fchamberWalls;
    
    // adding source assembly
    G4LogicalVolume*  fAlBackPlate;
    G4LogicalVolume*  fScrew1;
    G4LogicalVolume*  fScrew2;
    G4LogicalVolume*  fScrew3;
    
    G4LogicalVolume*  fSourceBody;
    G4LogicalVolume*  fSource;
    
  private:
    virtual void ConstructSDandField();
    
    DetectorMaterials * detMat;
    B1DetectorConstructionMessenger* fDetMessenger;
    
    // tracking things that change in geometry
    G4double  fAlphaHeight;
    G4bool    fAlphaStatus; // false means no source
    
    G4double  fGammaHeight;
    G4bool    fGammaStatus;
    
    G4bool    fCollStatus;
    
    G4String fGammaSource;
    
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

