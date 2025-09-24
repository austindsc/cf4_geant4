#include "SensitiveSIPM.hh"
#include "G4SystemOfUnits.hh"
#include "G4RunManager.hh"
#include "Randomize.hh"
#include "G4TransportationManager.hh"
#include "G4Navigator.hh"
#include "G4OpticalPhoton.hh"
#include "G4DynamicParticle.hh"
#include "G4TrackingManager.hh"
#include "G4SteppingManager.hh"
#include "G4TrackVector.hh"
#include "G4EventManager.hh"

#include "CF4EventAction.hh"
#include "MyGlobals.hh"


SensitiveSIPM::SensitiveSIPM(G4String name) : G4VSensitiveDetector(name) { 
  fEventAction = (CF4EventAction *) G4RunManager::GetRunManager()->GetUserEventAction();
  fhc = CLHEP::h_Planck * CLHEP::c_light;
  fLastTrackID = -1;

  // Initialize lookup table system
  fUse2DLookupTables = false;
  fLookupTablesLoaded = false;
  
  // Set default file paths
  fOpticsDataFile = "dataPDE/optics_HPK-VUV4_wWindow_17nmOxide_defaultData_globalPar_160k.csv";
  fPDEDataFile = "dataPDE/effective_internalPDE.txt";
  
  // Load all lookup tables with defaults
  LoadOpticsTable(fOpticsDataFile, 5, 3); // Rcol=5, Tcol=3
}

SensitiveSIPM::~SensitiveSIPM()
{}

void SensitiveSIPM::LoadOpticsTable(const G4String& opticsFile, int Rcol, int Tcol) {
  G4cout << "SensitiveSIPM: Loading 2D optical lookup tables from \n" << opticsFile << G4endl;
  
  // Use TwoDLookupTable as a helper to load data, then extract the TH2D
  TwoDLookupTable tempReflectivity("SiPM_Reflectivity");
  TwoDLookupTable tempTransmission("SiPM_Transmission");
  
  bool success = true;
  
  // Load reflectivity data
  if (!tempReflectivity.LoadFromCSVComplex(opticsFile, 0, 1, Rcol, true, ',', true)) {
    G4cout << "SensitiveSIPM: Failed to load reflectivity from " << opticsFile << G4endl;
    success = false; exit(1); // critical error
  } else {
    // Simply clone the histogram (ROOT manages memory)
    fReflectivityTable.reset(static_cast<TH2D*>(tempReflectivity.GetHistogram()->Clone("SiPM_Reflectivity")));
  }
  
  // Load transmission data
  if (!tempTransmission.LoadFromCSVComplex(opticsFile, 0, 1, Tcol, true, ',', true)) {
    G4cout << "SensitiveSIPM: Failed to load transmission from " << opticsFile << G4endl;
    success = false; exit(1); // critical error
  } else {
    // Simply clone the histogram (ROOT manages memory)
    fTransmissionTable.reset(static_cast<TH2D*>(tempTransmission.GetHistogram()->Clone("SiPM_Transmission")));
  }
  
  if (success) {
    // Store ranges for validation
    fWavelengthRange = {fReflectivityTable->GetXaxis()->GetXmin(), fReflectivityTable->GetXaxis()->GetXmax()};
    fAngleRange = {fReflectivityTable->GetYaxis()->GetXmin(), fReflectivityTable->GetYaxis()->GetXmax()};
    
    fLookupTablesLoaded = true;
    fUse2DLookupTables = true; // Enable usage
    G4cout << "SensitiveSIPM: 2D lookup tables loaded successfully from " << opticsFile << G4endl;
    
    // Print info
    if (MG::GetVerbosity() >= 1) {
      G4cout << "  Wavelength range: " << fWavelengthRange.first << " - " << fWavelengthRange.second << " nm" << G4endl;
      G4cout << "  Angle range: " << fAngleRange.first << " - " << fAngleRange.second << " degrees" << G4endl;
      G4cout << "  Reflectivity table: " << fReflectivityTable->GetNbinsX() << "x" << fReflectivityTable->GetNbinsY() << " bins" << G4endl;
      G4cout << "  Transmission table: " << fTransmissionTable->GetNbinsX() << "x" << fTransmissionTable->GetNbinsY() << " bins" << G4endl;
    }
  } else {
    fLookupTablesLoaded = false;
    fUse2DLookupTables = false;
    G4cout << "SensitiveSIPM: 2D lookup tables disabled due to loading errors" << G4endl;
  }
}

G4bool SensitiveSIPM::ProcessHits(G4Step * step, G4TouchableHistory * /*ROhist*/){
  // ProcessHits now only handles photons that made it inside the SiPM
  // Surface interactions are handled by HandlePhotonAtBoundary() called from SteppingAction
  
  G4Track * track = step->GetTrack();
  
  // Always count photon touching the SiPM surface (any step in sensitive volume)
  this->CountPhotonTouching(track->GetTrackID());
  
  // For photons that made it inside, handle detection
  track->SetTrackStatus(fStopAndKill);
  this->CountPhotonIn(step);
  return true;
}

// NEW METHOD: Handle surface optics at boundary (called from SteppingAction)
G4bool SensitiveSIPM::HandlePhotonAtBoundary(const G4Step* step) {
  G4Track* track = step->GetTrack();
  
  // Always count photon touching the SiPM surface
  this->CountPhotonTouching(track->GetTrackID());
  
  // Check if we're using 2D lookup tables for surface interactions
  if (fUse2DLookupTables && fLookupTablesLoaded) {
    return ProcessPhotonAtSurface(step);
  }
  
  // Fallback: kill photon at surface
  track->SetTrackStatus(fStopAndKill);
  return true;
}

// do a bit of logic here to avoid double counting twice reflected photons
inline G4bool SensitiveSIPM::CountPhotonTouching(G4int trackID){
  // G4cout << "in photon Touching" << G4endl;
  if(trackID == fLastTrackID) return false;
  else{
    fLastTrackID = trackID;
    fEventAction->AddTouchingPE();
    return true;
  }
}

inline G4bool SensitiveSIPM::CountPhotonIn(const G4Step * step){
  G4StepPoint * preStepPoint = step->GetPreStepPoint();
  G4ThreeVector pPhoton = preStepPoint->GetMomentum();
  G4double wavelength = fhc / pPhoton.mag() / nm; // wavelength in nm

  fEventAction->AddTruthPE(); // photon entered
  if(MG::GetVerbosity() >= 2) { 
    G4cout << "MC truth Hit! wl = " << wavelength << " nm" << G4endl; 
  }

  // Use internal PDE lookup table if loaded, otherwise default efficiency
  G4double pdeEfficiency = 0.25; // fallback default
  if (fInternalPDETable) {
    // Sample PDE at normal incidence (0 degrees)
    pdeEfficiency = fInternalPDETable->Interpolate(wavelength, 0.0);
  }

  if(G4UniformRand() < pdeEfficiency){
    if(MG::GetVerbosity() >= 2) {
      G4cout << "PDE hit! wl = " << wavelength << " nm, PDE = " << pdeEfficiency << G4endl;
    }
    fEventAction->AddDetPE();
  }
  return true;
}

// use this eventually, if datafiles become macro input
// void SensitiveSIPM::SetDataFiles(const G4String& opticsFile, const G4String& pdeFile) {
//   fOpticsDataFile = opticsFile;
//   fPDEDataFile = pdeFile;
  
//   // Reload tables with new file paths
//   LoadAllTables();
// }

G4bool SensitiveSIPM::ProcessPhotonAtSurface(const G4Step* step) {
  G4Track* track = step->GetTrack();
  G4StepPoint* preStepPoint = step->GetPreStepPoint();
  G4ThreeVector photonMomentum = preStepPoint->GetMomentum();
  
  // Calculate wavelength (in nm) and incident angle (in degrees)
  G4double wavelength = fhc / photonMomentum.mag() / nm;
  G4ThreeVector surfaceNormal = GetSurfaceNormal(step);
  G4double cosTheta = std::abs(photonMomentum.unit().dot(surfaceNormal));
  G4double incidentAngle = std::acos(cosTheta) * 180.0 / CLHEP::pi;
  
  if (MG::GetVerbosity() >= 2) {
    G4cout << "SensitiveSIPM: Photon at surface - wavelength = " << wavelength << " nm, "
           << "incident angle = " << incidentAngle << " degrees" << G4endl;
  }
  
  // STEP 1: Fill Factor test - define geometric active area fraction
  G4double fillFactor = 1.0; // 70% active area - can be made configurable later
  G4double ffRand = G4UniformRand();
  
  if (ffRand > fillFactor) {
    // Photon hits inactive area - kill it
    track->SetTrackStatus(fStopAndKill);
    if (MG::GetVerbosity() >= 2) {
      G4cout << "SensitiveSIPM: Photon hit inactive area (FF=" << fillFactor << ")" << G4endl;
    }
    return true;
  }
  
  // STEP 2: Photon hits active area - do R/T throws
  G4double reflectivity = 1.0; // fallback
  G4double transmission = 0.5; // fallback
  
  // if (fReflectivityTable) { //xx
  //   reflectivity = fReflectivityTable->Interpolate(wavelength, incidentAngle);
  // }
  // if (fTransmissionTable) {
  //   transmission = fTransmissionTable->Interpolate(wavelength, incidentAngle);
  // }
  
  G4double rtRand = G4UniformRand();

  if (rtRand < reflectivity) { ////// Reflected!
    // STEP 3a: REFLECT the photon specularly
    if (DoSpecularReflection(step)) {
      if (MG::GetVerbosity() >= 2) { G4cout << "SensitiveSIPM: Photon reflected specularly (R=" << reflectivity << ")" << G4endl;}
      return true;
    } else {
      // Reflection failed for some reason? - kill photon
      track->SetTrackStatus(fStopAndKill);
      G4cout << "SensitiveSIPM: Photon reflection failed, killing photon" << G4endl;
      return true;
    }

  } else if (rtRand < reflectivity + transmission) { ///// Transmitted!
    // STEP 3b: TRANSMIT and continue to detection
    track->SetTrackStatus(fStopAndKill);
    this->CountPhotonIn(step);

    // this->DoInternalPDE(wavelength); // xx
    
    if (MG::GetVerbosity() >= 2) {
      G4cout << "SensitiveSIPM: Photon transmitted for detection (T=" << transmission << ")" << G4endl;
    }
    return true;

  } else { ////// Absorbed!
    // ABSORBED in surface layers
    track->SetTrackStatus(fStopAndKill);
    if (MG::GetVerbosity() >= 2) {
      G4cout << "SensitiveSIPM: Photon absorbed at surface" << G4endl;
    }
    return true;
  }
}

// Helper function to do complete specular reflection with Geant4
G4bool SensitiveSIPM::DoSpecularReflection(const G4Step* step) {
  G4Track* track = step->GetTrack();
  G4StepPoint* preStepPoint = step->GetPreStepPoint();
  
  // Get surface normal and calculate reflection direction
  G4ThreeVector surfaceNormal = GetSurfaceNormal(step);
  G4ThreeVector photonMomentum = preStepPoint->GetMomentum();
  G4ThreeVector incidentDirection = photonMomentum.unit();
  G4ThreeVector newDirection = ReflectPhoton(incidentDirection, surfaceNormal);
  
  if (MG::GetVerbosity() >= 2) {
    G4cout << "SensitiveSIPM: Reflecting photon direction from (" 
           << incidentDirection.x() << ", " << incidentDirection.y() << ", " << incidentDirection.z() 
           << ") to (" << newDirection.x() << ", " << newDirection.y() << ", " << newDirection.z() << ")" << G4endl;
  }
  
  // Update track with reflected direction
  track->SetMomentumDirection(newDirection);
  
  // Move photon slightly back into the CF4 medium to avoid boundary issues
  G4ThreeVector currentPos = track->GetPosition();
  // Move MUCH further back to avoid immediate re-reflection (5mm instead of 1mm)
  G4ThreeVector newPos = currentPos + 5*mm * surfaceNormal; 
  track->SetPosition(newPos);
  
  // CRITICAL: Force Geant4 navigator to relocate the photon to update material
  G4Navigator* navigator = G4TransportationManager::GetTransportationManager()
                          ->GetNavigatorForTracking();
  G4VPhysicalVolume* newVolume = navigator->LocateGlobalPointAndSetup(newPos);
  
  // Force the track to update its material by updating the step point
  G4StepPoint* postStepPoint = const_cast<G4StepPoint*>(step->GetPostStepPoint());
  if (newVolume && postStepPoint) {
    postStepPoint->SetMaterial(newVolume->GetLogicalVolume()->GetMaterial());
    postStepPoint->SetMaterialCutsCouple(newVolume->GetLogicalVolume()->GetMaterialCutsCouple());
    
    // Note: G4Track doesn't have SetNextMaterial methods, so we rely on step point updates
  }
  
  if (MG::GetVerbosity() >= 2) {
    G4cout << "SensitiveSIPM: Surface normal: (" 
           << surfaceNormal.x() << ", " << surfaceNormal.y() << ", " << surfaceNormal.z() << ")" << G4endl;
    G4cout << "SensitiveSIPM: Moving from (" << currentPos.x() << ", " << currentPos.y() << ", " << currentPos.z() 
           << ") to (" << newPos.x() << ", " << newPos.y() << ", " << newPos.z() << ")" << G4endl;
    if (newVolume) {
      G4cout << "SensitiveSIPM: Photon relocated to volume: " << newVolume->GetName() << G4endl;
      G4Material* material = newVolume->GetLogicalVolume()->GetMaterial();
      if (material) {
        G4cout << "SensitiveSIPM: New material should be: " << material->GetName() << G4endl;
      }
      G4cout << "SensitiveSIPM: Track material after update: " << track->GetMaterial()->GetName() << G4endl;
    }
  }
  
  // Force the track to continue tracking after reflection
  track->SetGoodForTrackingFlag(true);
  track->SetTrackStatus(fAlive); // Make sure track continues
  
  if (MG::GetVerbosity() >= 2) {
    G4cout << "SensitiveSIPM: Photon reflected and repositioned - continuing tracking" << G4endl;
  }
  
  return true;
}

// Helper function to reflect photon direction about surface normal
G4ThreeVector SensitiveSIPM::ReflectPhoton(const G4ThreeVector& incident, const G4ThreeVector& normal) {
  // Perfect specular reflection: R = I - 2(I·N)N
  // where I is incident direction, N is surface normal, R is reflected direction
  G4double dot = incident.dot(normal);
  return incident - 2.0 * dot * normal;
}


// Helper function to get surface normal from geometry
G4ThreeVector SensitiveSIPM::GetSurfaceNormal(const G4Step* step) {
  // For SiPM surface, we want the normal pointing outward from SiPM into CF4
  // Let's use the transformation method first as it's more predictable
  
  G4TouchableHandle touchable = step->GetPreStepPoint()->GetTouchableHandle();
  if (touchable) {
    G4AffineTransform transform = touchable->GetHistory()->GetTopTransform();
    
    // Assume local surface normal is +Z in SiPM's local coordinates (pointing outward)
    G4ThreeVector localNormal(0, 0, 1);
    G4ThreeVector globalNormal = transform.TransformAxis(localNormal);
    
    // Fix the Y coordinate sign issue
    globalNormal.setY(-globalNormal.y());
    
    if (MG::GetVerbosity() >= 2) {
      G4cout << "SensitiveSIPM: Using transformed surface normal (Y-corrected): " 
             << "(" << globalNormal.x() << ", " << globalNormal.y() << ", " << globalNormal.z() << ")" << G4endl;
    }
    return globalNormal.unit();
  }
  
  // Try geometry method as fallback
  G4StepPoint* postPoint = step->GetPostStepPoint();
  if (postPoint->GetStepStatus() == fGeomBoundary) {
    G4Navigator* navigator = G4TransportationManager::GetTransportationManager()
                            ->GetNavigatorForTracking();
    G4bool valid;
    G4ThreeVector surfaceNormal = navigator->GetGlobalExitNormal(postPoint->GetPosition(), &valid);
    
    if (valid) {
      // Check if this normal makes sense (should point roughly +Z for SiPM surface)
      if (surfaceNormal.z() < 0) {
        surfaceNormal = -surfaceNormal; // Flip if pointing wrong way
      }
      
      if (MG::GetVerbosity() >= 2) {
        G4cout << "SensitiveSIPM: Using geometry surface normal: " 
               << "(" << surfaceNormal.x() << ", " << surfaceNormal.y() << ", " << surfaceNormal.z() << ")" << G4endl;
      }
      return surfaceNormal.unit();
    }
  }
  
  // Final fallback - assumes SiPM surface faces +Z direction
  G4cout << "SensitiveSIPM: Warning - using default +Z surface normal" << G4endl;
  return G4ThreeVector(0, 0, 1);
}