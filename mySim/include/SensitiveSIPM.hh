#ifndef SensitiveSIPM_H
#define SensitiveSIPM_H

#include "G4VSensitiveDetector.hh"
#include "TwoDLookupTable.hh"
#include "TH2D.h"
#include <memory>

class G4UniformRand;
class CF4EventAction;

class SensitiveSIPM : public G4VSensitiveDetector
{
public:
  SensitiveSIPM(G4String name);
  ~SensitiveSIPM();
  
  // Main processing method - handles all photon interactions
  G4bool ProcessHits(G4Step * step, G4TouchableHistory *);
  
  // Tracking behavior
  inline G4bool CountPhotonTouching(G4int trackID);
  inline G4bool CountPhotonIn(const G4Step * step);
  void ResetForFirstEvent(){fLastTrackID = -1;}

  // Control methods
  // void SetDataFiles(const G4String& opticsFile, const G4String& pdeFile = "");
  void SetUse2DLookupTables(G4bool use) { fUse2DLookupTables = use; }
  G4bool GetUse2DLookupTables() const { return fUse2DLookupTables; }
  
  // Surface optics method called from SteppingAction
  G4bool HandlePhotonAtBoundary(const G4Step* step);
  
private:
  CF4EventAction * fEventAction;
  G4int fLastTrackID;
  G4double fhc;

  // File paths for data tables
  G4String fOpticsDataFile;
  G4String fPDEDataFile;

  // 2D lookup tables as TH2D objects - loaded automatically in constructor
  std::unique_ptr<TH2D> fReflectivityTable;
  std::unique_ptr<TH2D> fTransmissionTable;
  std::unique_ptr<TH2D> fInternalPDETable;
  
  G4bool fUse2DLookupTables;
  G4bool fLookupTablesLoaded;

  // Helper methods
  G4bool ProcessPhotonAtSurface(const G4Step* step);
  void LoadOpticsTable(const G4String& opticsFile = "", int Rcol = 5, int Tcol = 3);
  void LoadInternalPDETable(const G4String& pdeFile = "");
  
  std::pair<G4double, G4double> fWavelengthRange; // {min_wlen, max_wlen}
  std::pair<G4double, G4double> fAngleRange;      // {min_angle, max_angle}
  
  // Geometry and reflection helpers
  G4ThreeVector GetSurfaceNormal(const G4Step* step);
  G4ThreeVector ReflectPhoton(const G4ThreeVector& incident, const G4ThreeVector& normal);
  G4bool DoSpecularReflection(const G4Step* step);

};

#endif
