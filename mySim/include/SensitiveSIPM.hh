#ifndef SensitiveSIPM_H
#define SensitiveSIPM_H

#include "G4VSensitiveDetector.hh"
#include "G4PhysicsFreeVector.hh"

class G4UniformRand;
class CF4EventAction;
class SiPMOpticsManager;

class SensitiveSIPM : public G4VSensitiveDetector
{
public:
  SensitiveSIPM(G4String);
  ~SensitiveSIPM();  
  G4PhysicsFreeVector * ovecPDE;
  
  G4bool ProcessHits(G4Step * step, G4TouchableHistory *);
  
  inline G4bool CountPhotonTouching(G4int trackID);
  inline G4bool CountPhotonIn(const G4Step * step);
  void ResetForFirstEvent(){fLastTrackID = -1;}

  // New methods for 2D lookup table support
  void LoadOpticsLookupTables(const G4String& reflectivityFile = "", 
                             const G4String& transmissionFile = "");
  void SetUse2DLookupTables(G4bool use) { fUse2DLookupTables = use; }
  G4bool GetUse2DLookupTables() const { return fUse2DLookupTables; }
  
private:
  CF4EventAction * fEventAction;
  G4int fLastTrackID; // use this to track photons from the same track (avoid double counting reflections)
  G4double fhc;

  // 2D lookup table support
  SiPMOpticsManager* fOpticsManager;
  G4bool fUse2DLookupTables;
  G4bool fLookupTablesLoaded;

  // Helper methods for optical processes
  G4bool ProcessPhotonAtSurface(const G4Step* step);
  G4double CalculateReflectivity(G4double wavelength, G4double incidentAngle);
  G4double CalculateTransmission(G4double wavelength, G4double incidentAngle);

};

#endif
