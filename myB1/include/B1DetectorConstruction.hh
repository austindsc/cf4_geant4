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
    
    G4double GetAlphaHeight() { return fSourceHeight; }
    void SetAlphaHeight(G4double h) { fSourceHeight = h;}

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
    G4double fSourceHeight;
    
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

