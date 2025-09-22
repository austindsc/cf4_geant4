#include "SensitiveSIPM.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicsOrderedFreeVector.hh"
#include "G4RunManager.hh"
#include "Randomize.hh"

#include "CF4EventAction.hh"
#include "HelperFunctions.hh"
#include "MyGlobals.hh"

// Include the 2D lookup table system
#include "SiPMOpticsManager.hh"
#include "SiPMOpticsHelpers.hh"

#include <filesystem>

SensitiveSIPM::SensitiveSIPM(G4String name) : G4VSensitiveDetector(name )
{ 
  fEventAction = (CF4EventAction *) G4RunManager::GetRunManager()->GetUserEventAction();
  fhc = CLHEP::h_Planck * CLHEP::c_light;
  fLastTrackID = -1; // so don't miss first track

  // Initialize 2D lookup table system
  fOpticsManager = SiPMOpticsManager::GetInstance();
  fUse2DLookupTables = false; // Default to off until tables are loaded
  fLookupTablesLoaded = false;

  // adding PDE
  ovecPDE = new G4PhysicsFreeVector(false);
  
  G4double scaleX, scaleY;
  // using wavelengths, not energy, is okay. Do the efficiency call on my own
  std::string fname = "dataOptics/effective_internalPDE.txt";
  OH::LoadTxt_OrderedVector(fname, ovecPDE, MG::GetVerbosity(), scaleX = nm, scaleY = 1.);
  if(MG::GetVerbosity() > 1) {
    G4cout << "dumping PDE values from\n" << fname << G4endl; ovecPDE->DumpValues();

    G4cout << 
    "------------------------ SiPM Implementation: enforce Absorption in Si ------------------------\n"
    "using normal ProcessHits in sensitive detector. \n"
    "Must define ABSLENGTH as short in SiBulk. Then ProcessHits() checks if pre and post volumes are equal"
    "if they are equal it kills the track and triggers 'count PhotonIn()\n"
    "------------------------  ------------------------" << G4endl;
  }

  // G4cout << "PDE at 199.975 nm: " << ovecPDE->Value(199.975*nm) << G4endl;
  // exit(0);
  return;
}

SensitiveSIPM::~SensitiveSIPM()
{}

G4bool SensitiveSIPM::ProcessHits(G4Step * step, G4TouchableHistory *ROhist){
  G4Track * track = step->GetTrack();
  this->CountPhotonTouching(track->GetTrackID()); // if photon touches the surface of the SiPM
  
  // Check if we're using 2D lookup tables for surface interactions
  if (fUse2DLookupTables && fLookupTablesLoaded) {
    // Handle surface reflection/transmission with lookup tables
    G4VPhysicalVolume* physPreVolume = step->GetPreStepPoint()->GetPhysicalVolume();
    G4VPhysicalVolume* physPostVolume = step->GetPostStepPoint()->GetPhysicalVolume();
    
    // If photon is at the surface (different volumes), apply lookup table logic
    if (physPreVolume != physPostVolume) {
      return ProcessPhotonAtSurface(step);
    }
  }
  
  // Original logic: kill photon once inside Si (requires ABSLength short) 
  // and both pre and post steps are equal
  // this is option 2 from PDE study 
  G4VPhysicalVolume* physPreVolume = step->GetPreStepPoint()->GetPhysicalVolume();
  G4VPhysicalVolume* physPostVolume = step->GetPostStepPoint()->GetPhysicalVolume();
  if(physPreVolume == physPostVolume){
    track->SetTrackStatus(fStopAndKill);
    this->CountPhotonIn(step);
    return true;
  }
  return false;
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
  G4double wlen = fhc/pPhoton.mag(); //wavelength in nanometers 

  fEventAction->AddTruthPE(); // photon entered
  if(MG::GetVerbosity() >= 2) { G4cout << "MC truth Hit! wl = " << wlen/nm << " nm" << G4endl; }

  // With effective internal Efficiency
  if(G4UniformRand() < ovecPDE->Value(wlen)){
    if(MG::GetVerbosity() >= 2)G4cout << "PDE hit! wl = " << wlen/nm << " nm" <<G4endl;
    fEventAction->AddDetPE();
  }
  return true;
}

void SensitiveSIPM::LoadOpticsLookupTables(const G4String& reflectivityFile, 
                                          const G4String& transmissionFile) {
  
  G4cout << "SensitiveSIPM: Loading 2D optical lookup tables..." << G4endl;
  
  bool success = true;
  
  if (!reflectivityFile.empty()) {
    if (!fOpticsManager->LoadReflectivityTable(reflectivityFile)) {
      G4cerr << "SensitiveSIPM: Failed to load reflectivity table from " << reflectivityFile << G4endl;
      success = false;
    }
  }
  
  if (!transmissionFile.empty()) {
    if (!fOpticsManager->LoadTransmissionTable(transmissionFile)) {
      G4cerr << "SensitiveSIPM: Failed to load transmission table from " << transmissionFile << G4endl;
      success = false;
    }
  }
  
  if (success && (!reflectivityFile.empty() || !transmissionFile.empty())) {
    fLookupTablesLoaded = true;
    fUse2DLookupTables = true; // Enable usage
    G4cout << "SensitiveSIPM: 2D lookup tables loaded successfully and enabled" << G4endl;
    fOpticsManager->PrintAllTables();
  } else {
    fLookupTablesLoaded = false;
    fUse2DLookupTables = false;
    G4cout << "SensitiveSIPM: 2D lookup tables disabled due to loading errors" << G4endl;
  }
}

G4bool SensitiveSIPM::ProcessPhotonAtSurface(const G4Step* step) {
  G4Track* track = step->GetTrack();
  G4StepPoint* preStepPoint = step->GetPreStepPoint();
  G4ThreeVector photonMomentum = preStepPoint->GetMomentum();
  
  // Calculate wavelength and incident angle
  G4double wavelength = SiPMOpticsHelpers::CalculateWavelength(photonMomentum);
  G4ThreeVector surfaceNormal = SiPMOpticsHelpers::GetSiPMSurfaceNormal(); // You may need to adapt this
  G4double incidentAngle = SiPMOpticsHelpers::CalculateIncidentAngle(photonMomentum, surfaceNormal);
  
  if (MG::GetVerbosity() >= 2) {
    G4cout << "SensitiveSIPM: Photon at surface - wavelength = " << wavelength << " nm, "
           << "incident angle = " << incidentAngle << " degrees" << G4endl;
  }
  
  // Sample reflectivity from lookup table
  G4double reflectivity = CalculateReflectivity(wavelength, incidentAngle);
  
  // Make reflection/transmission decision
  G4double randomValue = G4UniformRand();
  G4bool shouldReflect = SiPMOpticsHelpers::ShouldReflect(reflectivity, randomValue);
  
  if (shouldReflect) {
    // Photon is reflected - in a full implementation, you would:
    // 1. Change the photon's momentum direction (reflect about surface normal)
    // 2. Continue the track
    // For now, we'll just kill it and count as not detected
    if (MG::GetVerbosity() >= 2) {
      G4cout << "SensitiveSIPM: Photon reflected (R=" << reflectivity << ")" << G4endl;
    }
    track->SetTrackStatus(fStopAndKill);
    return true;
  } else {
    // Photon is transmitted - proceed to detection
    G4double transmission = CalculateTransmission(wavelength, incidentAngle);
    if (MG::GetVerbosity() >= 2) {
      G4cout << "SensitiveSIPM: Photon transmitted (T=" << transmission << ")" << G4endl;
    }
    
    // Apply transmission probability
    if (G4UniformRand() < transmission) {
      track->SetTrackStatus(fStopAndKill);
      this->CountPhotonIn(step);
      return true;
    } else {
      // Absorbed in surface layers, not detected
      track->SetTrackStatus(fStopAndKill);
      return true;
    }
  }
}

G4double SensitiveSIPM::CalculateReflectivity(G4double wavelength, G4double incidentAngle) {
  if (fOpticsManager->HasReflectivityTable()) {
    return fOpticsManager->SampleReflectivity(wavelength, incidentAngle, true);
  } else {
    // Fallback to Fresnel calculation (you may need to adjust refractive indices)
    G4double n1 = 1.0; // Air/vacuum
    G4double n2 = 3.5; // Silicon (approximate)
    return SiPMOpticsHelpers::CalculateFresnelReflectance(n1, n2, incidentAngle);
  }
}

G4double SensitiveSIPM::CalculateTransmission(G4double wavelength, G4double incidentAngle) {
  if (fOpticsManager->HasTransmissionTable()) {
    return fOpticsManager->SampleTransmission(wavelength, incidentAngle, true);
  } else {
    // Fallback: transmission = 1 - reflectivity (simplified)
    return 1.0 - CalculateReflectivity(wavelength, incidentAngle);
  }
}