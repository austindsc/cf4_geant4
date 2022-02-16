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
/// \file B1DetectorConstruction.hh
/// \brief Definition of the B1DetectorConstruction class

#ifndef B1DetectorConstruction_h
#define B1DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;
class B1DetectorConstructionMessenger;

/// Detector construction class to define materials and geometry.
class B1DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    B1DetectorConstruction();
    virtual ~B1DetectorConstruction();

    virtual G4VPhysicalVolume* Construct();
    
    G4LogicalVolume* GetScoringVolume() const { return fScoringVolume; }
    
    G4double getSourceHeight(){ return fSourceHeight; }
    void setSourceHeight(G4double h){ fSourceHeight = h; }

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
    
    // tracking things that change in geometry
    G4double fSourceHeight;
    
  private:
    B1DetectorConstructionMessenger* fDetMessenger;
    
    
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

