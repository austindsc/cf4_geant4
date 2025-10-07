#include "SiPMSurfaceWLS.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VSensitiveDetector.hh"
#include "G4StepPoint.hh"
#include "G4TransportationManager.hh"
#include "G4SDManager.hh"
#include "Randomize.hh"
#include "MyGlobals.hh"
#include "CLHEP/Units/PhysicalConstants.h"
#include "TwoDLookupTable.hh"
#include <cmath>

// Static member initialization
SiPMSurfaceWLS* SiPMSurfaceWLS::fInstance = nullptr;

//==============================================================================
// Constructor - inherits from G4OpBoundaryProcess
//==============================================================================
SiPMSurfaceWLS::SiPMSurfaceWLS(const G4String& processName)
    : G4OpBoundaryProcess(processName), 
      fInvokeSD(true),
      fTransmissionHist(nullptr),
      fReflectionHist(nullptr),
      fInternalPDEHist(nullptr),
      fFillFactor(1.0),
      fRdiffuseFraction(0.),
      fVoltage(4.0) {  // Default 4V

    fInstance = this;
    
    G4cout << "SiPMSurfaceWLS: Initializing enhanced boundary process..." << G4endl;
    
    // Load optical data tables
    SetupSiPMType("HPK-VUV4"); // Hardcoded for now - change as needed
    fDoShadowing = true; // enable shadowing by default
    if(fDoShadowing){
        fW = W_from_FF(fFillFactor); // W = ff/p
        fH = fResistorHeight / fPixelPitch; // H = h/p
    }
    G4bool mandatory;
    LoadOpticsTable(mandatory=true);
    LoadInternalPDETable(mandatory=true);

    ////// Assign range variables after tables are loaded //////
    if (fTransmissionHist && fReflectionHist) {
        // Use bin centers instead of bin edges for proper interpolation range
        G4int nXbins = fTransmissionHist->GetNbinsX();
        G4int nYbins = fTransmissionHist->GetNbinsY();
        
        fOptics_wavelengthRange = std::make_pair(
            fTransmissionHist->GetXaxis()->GetBinCenter(1),        // First bin center
            fTransmissionHist->GetXaxis()->GetBinCenter(nXbins-1)    // Last bin center
        );
        fOptics_angleRange = std::make_pair(
            fTransmissionHist->GetYaxis()->GetBinCenter(1),        // First bin center
            fTransmissionHist->GetYaxis()->GetBinCenter(nYbins-1)    // Last bin center
        );
        if (MG::GetVerbosity() >= 1) {
        G4cout << "Optics wavelength range: " << fOptics_wavelengthRange.first 
               << " - " << fOptics_wavelengthRange.second << " nm" << G4endl;
        G4cout << "Optics angle range: " << fOptics_angleRange.first 
               << " - " << fOptics_angleRange.second << " deg" << G4endl;
        }
    }
    if (fInternalPDEHist) {
        // Use bin centers instead of bin edges for proper interpolation range
        G4int nXbins_pde = fInternalPDEHist->GetNbinsX();
        G4int nYbins_pde = fInternalPDEHist->GetNbinsY();
        
        fInternalPDE_wlenRange = std::make_pair(
            fInternalPDEHist->GetXaxis()->GetBinCenter(1),         // First bin center
            fInternalPDEHist->GetXaxis()->GetBinCenter(nXbins_pde-1) // Last bin center
        );
        fInternalPDE_voltageRange = std::make_pair(
            fInternalPDEHist->GetYaxis()->GetBinCenter(1),         // First bin center
            fInternalPDEHist->GetYaxis()->GetBinCenter(nYbins_pde-1) // Last bin center
        );
        if (MG::GetVerbosity() >= 1) {
        G4cout << "Internal PDE wavelength range: " << fInternalPDE_wlenRange.first 
               << " - " << fInternalPDE_wlenRange.second << " nm" << G4endl;
        G4cout << "Internal PDE voltage range: " << fInternalPDE_voltageRange.first 
               << " - " << fInternalPDE_voltageRange.second << " V" << G4endl;
        }
    }

    // G4cout << "R value:" << fReflectionHist->Interpolate(200, 20) << G4endl; // test interpolation to ensure loaded
    // G4cout << fTransmissionHist->GetNbinsX() << " x " << fTransmissionHist->GetNbinsY() << " bins" << G4endl;
    // G4cout << "T value:" << fTransmissionHist->Interpolate(200, 20) << G4endl;

}

SiPMSurfaceWLS::~SiPMSurfaceWLS() {
    // Clean up the copied histograms
    delete fTransmissionHist;
    delete fReflectionHist;  
    delete fInternalPDEHist;
    
    if (fInstance == this) {
        fInstance = nullptr;
    }
}

void SiPMSurfaceWLS::SetupSiPMType(G4String type) {
    if (type == "HPK-VUV4") {
        fOpticsDataFile = "dataPDE/optics_HPK-VUV4_noWindow_17nmOxide_defaultData_globalPar_160k.csv";
        fInternalPDEFile = "dataPDE/internalPDE_HPK-VUV4_noWindow_17nmOxide_defaultData_globalPar_160k.csv";
        fFillFactor = 0.6;              // Example fill factor
        fRdiffuseFraction = 0.17;      // Example (total) diffuse reflection fraction

        fResistorHeight = 2.5;
        fPixelPitch = 50;

    } else if (type == "OtherSiPM") {
        // Set parameters for another SiPM type
        fOpticsDataFile = "dataPDE/optics_OtherSiPM.csv";
        fInternalPDEFile = "dataPDE/effective_internalPDE_OtherSiPM.txt";
        fFillFactor = 0.75;           // Example fill factor
        fRdiffuseFraction = 0.2;      // Example diffuse reflection fraction
    } else {
        G4cout << "SiPMSurfaceWLS: Unknown SiPM type '" << type << "'. Using default parameters." << G4endl;
    }
}

//==============================================================================
// Enhanced PostStepDoIt - custom SiPM logic + fallback to standard boundary
//==============================================================================
G4VParticleChange* SiPMSurfaceWLS::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) {
    // G4cout << "\n\n------\nPre Step status: " << aStep.GetPreStepPoint()->GetStepStatus() << G4endl;
    // G4cout << "Post Step status: " << aStep.GetPostStepPoint()->GetStepStatus() << G4endl;
    // G4cout << fGeomBoundary << G4endl;


    // if(aStep.GetPreStepPoint()->GetStepStatus() == fGeomBoundary) {
    //   // Photon is at a geometric boundary - perfect for surface optics
    //   G4cout << "Pre volume: " << aStep.GetPreStepPoint()->GetPhysicalVolume()->GetName() << G4endl;
    //   G4cout << "Post volume: " << aStep.GetPostStepPoint()->GetPhysicalVolume()->GetName() << G4endl;
    //   G4cout << "also in sensitive detector?: " << EnteringSensitiveDetector(aTrack, aStep) <<"\n-------\n" << G4endl;
    // }

    // EXTREME TEST: Don't even call G4OpBoundaryProcess - just kill photon
    // aParticleChange.Initialize(aTrack);
    // aParticleChange.ProposeTrackStatus(fStopAndKill);
    // return &aParticleChange;
    // G4cout << "SiPMSurfaceWLS: PostStepDoIt called" << G4endl;

    // this is us!
    // Check if we're entering a sensitive detector (SiPM)
    if (EnteringSensitiveDetector(aTrack, aStep)){
        if(MG::GetVerbosity() >= 2) G4cout << "SiPMSurfaceWLS: At SiPM boundary - applying custom optics" << G4endl;
        return DoCustomOpticsAndDetection(aTrack, aStep);
    }

    // Not at SiPM boundary - use standard G4OpBoundaryProcess behavior
    return G4OpBoundaryProcess::PostStepDoIt(aTrack, aStep); // going straight to here still gave seg fault
}

//==============================================================================
// Check if entering a sensitive detector volume
bool SiPMSurfaceWLS::EnteringSensitiveDetector(const G4Track& aTrack, const G4Step& aStep) {
    G4StepPoint* postStepPoint = aStep.GetPostStepPoint();
    G4VPhysicalVolume* nextVolume = postStepPoint->GetPhysicalVolume();

    const G4ParticleDefinition* particleType = aTrack.GetParticleDefinition();
    if (particleType != G4OpticalPhoton::OpticalPhoton()) {
        return false;
    } // Only care about optical photons

    if(!fSiPMDetector){
        fSiPMDetector = GetSiPMDetector(DETECTORSETUP::GetSiPMName());
    }

    G4bool boundaryStatus = (aStep.GetPreStepPoint()->GetStepStatus() == fGeomBoundary);
    
    if (nextVolume && boundaryStatus == fGeomBoundary) { // xx check this
        G4LogicalVolume* logicalVol = nextVolume->GetLogicalVolume();
        if (logicalVol && logicalVol->GetSensitiveDetector()) {

            // last check: is this the SiPM sensitive detector?
            G4String sdName = logicalVol->GetSensitiveDetector()->GetName();
            // G4cout << "DEBUG: Sensitive detector name: " << sdName << G4endl;
            // G4cout << "DEBUG: fSiPMDetector name: " << fSiPMDetector->GetName() << G4endl;

            if (sdName == (G4String)fSiPMDetector->GetName()) {
                // G4cout << "SiPMSurfaceWLS: Photon entering sensitive detector volume: " << sdName << G4endl;
                return true;
            }
        }
    }
    return false;
}


// void SiPMSurfaceWLS::HandleRange(G4double& wlen, G4double & aoi, G4double &voltage){

// }

//==============================================================================
// Apply custom optics and possibly detection at SiPM surface
//==============================================================================
G4VParticleChange* SiPMSurfaceWLS::DoCustomOpticsAndDetection(const G4Track& aTrack, const G4Step& aStep) {

    aParticleChange.Initialize(aTrack);
    
    // Critical safety checks
    if (!fTransmissionHist) {
        G4cout << "ERROR: fTransmissionHist is null!" << G4endl;
        exit(1);
    }
    if (!fReflectionHist) {
        G4cout << "ERROR: fReflectionHist is null!" << G4endl;
        exit(1);
    }
    if (!fInternalPDEHist) {
        G4cout << "ERROR: fInternalPDEHist is null!" << G4endl;
        exit(1);
    }
    if (!fSiPMDetector){
        fSiPMDetector = GetSiPMDetector(DETECTORSETUP::GetSiPMName());
        if (!fSiPMDetector) {
          G4cout << "ERROR: Could not get SiPM detector!" << G4endl;
          exit(1);
        }
    } // should only get called once (to point towards the sensitive detector)
    
    fSiPMDetector->CountPhotonTouching(aTrack.GetTrackID());

    /////// -- doing math and setup -- ///////
    // Get current position and direction, wlen, do some math. photon energy in eV, then convert to wavelength in nm
    G4double wlen_nm = (CLHEP::h_Planck * CLHEP::c_light / aTrack.GetDynamicParticle()->GetTotalEnergy()) / CLHEP::nanometer;
    G4ThreeVector currentDir = aTrack.GetMomentumDirection().unit();
    G4ThreeVector normal = GetSurfaceNormal(aStep);
    G4double incidentAngle_deg = std::abs(currentDir.angle(-normal)) / CLHEP::deg; // angle between incident and normal in degrees
    if(MG::GetVerbosity() >= 2) {
        G4cout << "  Current AOI: " << incidentAngle_deg << " deg" << G4endl;
        G4cout << "  Current wavelength: " << wlen_nm << " nm" << G4endl;
    }

    //// -- check angle max range -- ////
    if (incidentAngle_deg > fOptics_angleRange.second) {
        incidentAngle_deg = fOptics_angleRange.second;
        if(MG::GetVerbosity() >= 2) {
            G4cout << "⚠️ SiPMSurfaceWLS: Incident angle " << incidentAngle_deg 
                   << " deg out of optics table range (" << fOptics_angleRange.first 
                   << " - " << fOptics_angleRange.second << " deg)." << G4endl;
        }
    }

    //// -- doing PDE decision tree -- ////
    if (G4UniformRand() < fFillFactor) { // on active area
        G4double shadowT = 1.0, shadowR = 1.0;

        if (fDoShadowing){
            shadowT = FF_theta_relative_transmission(incidentAngle_deg, fW, fH)/fFillFactor;
            shadowR = FF_theta_relative_reflection(incidentAngle_deg, fW, fH)/fFillFactor;
        }
        G4double transmission = shadowT * fTransmissionHist->Interpolate(wlen_nm, incidentAngle_deg);
        G4double reflection = shadowR * fReflectionHist->Interpolate(wlen_nm, incidentAngle_deg);

        if (G4UniformRand() < transmission) { // transmitted into SiPM
            fSiPMDetector->CountPhotonIn(wlen_nm, incidentAngle_deg);
            aParticleChange.ProposeTrackStatus(fStopAndKill); // could kill photon here, or let it continue into processhits()
            G4double internalPDE = fInternalPDEHist->Interpolate(wlen_nm, fVoltage);

            if(MG::GetVerbosity() >= 2) {
                G4cout << "SiPMSurfaceWLS: Photon transmitted into SiPM active area" << G4endl;
                G4cout << "  Transmission: " << transmission << G4endl;
                G4cout << "  Internal PDE: " << internalPDE << G4endl;
            }
            if(G4UniformRand() < internalPDE){ // detected internally
                // G4cout << "DEBUG: About to call CountPhotonDetected..." << G4endl;
                fSiPMDetector->CountPhotonDetected(wlen_nm, incidentAngle_deg);
                if(MG::GetVerbosity() >= 2) {
                    G4cout << "SiPMSurfaceWLS: Photon detected inside SiPM" << G4endl;
                    G4cout << "  voltage: " << fVoltage << G4endl;
                }
            }
            return &aParticleChange;
        } else if (G4UniformRand() < reflection) { // reflected off SiPM active surface, doing specular reflection
            G4ThreeVector specularVec = GetSpecularAngle(currentDir, normal).unit();
            aParticleChange.ProposeMomentumDirection(specularVec);
            if(MG::GetVerbosity() >= 2) G4cout << "SiPMSurfaceWLS: Photon specularly reflected off SiPM surface" << G4endl;
            return &aParticleChange;
        } else{ // absorbed on active surface (via R + T  < 1) which *may* be the case depending on physics table inputs
            if(MG::GetVerbosity() >= 2) G4cout << "SiPMSurfaceWLS: Photon absorbed at SiPM surface" << G4endl;
            aParticleChange.ProposeTrackStatus(fStopAndKill);
            return &aParticleChange;
        }
    } else { // on inactive area (1-FF)
        //diffuse reflectivity off inactive area
        if (G4UniformRand() < GetFFrelativeDiffuseFraction()) { // diffuse reflection off 1-FF
            if(MG::GetVerbosity() >= 2) G4cout << "SiPMSurfaceWLS: Photon hit dead area - diffusely reflecting" << G4endl;
            G4ThreeVector diffuseVec = GetDiffuseAngle(normal);
            aParticleChange.ProposeMomentumDirection(diffuseVec);
            return &aParticleChange;
        // photon dies on inactive surface (1-FF)    
        } else {
            if(MG::GetVerbosity() >= 2) G4cout << "SiPMSurfaceWLS: Photon hit dead area - absorbed" << G4endl;
            aParticleChange.ProposeTrackStatus(fStopAndKill);
            return &aParticleChange;
        }
    }    
    return &aParticleChange;
}

//==============================================================================
// Load transmission and reflection tables from main optics file
//==============================================================================
void SiPMSurfaceWLS::LoadOpticsTable(G4bool mandatory) {
    // Load transmission data (column 2)
    TwoDLookupTable* tempTransTable = new TwoDLookupTable("transmission");
    bool success1 = tempTransTable->LoadFromCSV(fOpticsDataFile, 0, 1, 3, true, ',');
    if (!success1) {
        G4cout << "❌ Warning: Failed to load transmission data from " << fOpticsDataFile << G4endl;
        fTransmissionHist = nullptr;
        if (mandatory) { exit(1);}
    } else {
        G4cout << "✅ Transmission table loaded" << G4endl;
        // Create a persistent copy of the histogram
        const TH2D* originalHist = tempTransTable->GetHistogram();
        fTransmissionHist = new TH2D(*originalHist);  // Copy constructor creates independent copy
    }
    
    // Load reflection data (column 4) 
    TwoDLookupTable* tempReflTable = new TwoDLookupTable("reflection");
    bool success2 = tempReflTable->LoadFromCSV(fOpticsDataFile, 0, 1, 5, true, ',');
    if (!success2) {
        G4cout << "❌ Warning: Failed to load reflection data from " << fOpticsDataFile << G4endl;
        fReflectionHist = nullptr;
        if (mandatory) { exit(1);}
    } else {
        G4cout << "✅ Reflection table loaded" << G4endl;
        // Create a persistent copy of the histogram
        const TH2D* originalHist = tempReflTable->GetHistogram();
        fReflectionHist = new TH2D(*originalHist);  // Copy constructor creates independent copy
    }
    
    // Clean up temporary table objects (keeping the histograms)
    delete tempTransTable;
    delete tempReflTable;
}
void SiPMSurfaceWLS::LoadInternalPDETable(G4bool mandatory) {
    // Load internal PDE data
    TwoDLookupTable* tempPDETable = new TwoDLookupTable("internalPDE");
    bool success = tempPDETable->LoadFromCSV(fInternalPDEFile, 0, 1, 2, true, ',');
    if (!success) {
        G4cout << "❌ Warning: Failed to load internal PDE data from " << fInternalPDEFile << G4endl;
        fInternalPDEHist = nullptr;
        if (mandatory) { exit(1);}
    } else {
        G4cout << "✅ Internal PDE table loaded" << G4endl;
        fInternalPDEHist = new TH2D(*tempPDETable->GetHistogram()); // make a copy for persistence
    }
    
    // Clean up temporary table object (keeping the histogram)
    delete tempPDETable;
}

//==============================================================================
// Get SensitiveSIPM detector by name from G4SDManager
//==============================================================================
SensitiveSIPM* SiPMSurfaceWLS::GetSiPMDetector(G4String sdName) {
    static SensitiveSIPM* cachedDetector = nullptr;
    
    if (!cachedDetector) {
        G4SDManager* sdManager = G4SDManager::GetSDMpointer();
        G4VSensitiveDetector* sd = sdManager->FindSensitiveDetector(sdName);
        cachedDetector = static_cast<SensitiveSIPM*>(sd);
        
        if (!cachedDetector) {
            G4cout << "Warning: Could not find SensitiveSIPM detector with name " << sdName << G4endl;
            G4cout << "Exiting..." << G4endl;
            exit(1);
        }
    }
    return cachedDetector;
}

//==============================================================================
// doing some math
//==============================================================================
G4ThreeVector SiPMSurfaceWLS::GetSurfaceNormal(const G4Step& aStep) {
    G4Navigator* navigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
    
    // GetGlobalExitNormal typically needs these parameters:
    G4ThreeVector localPoint = G4ThreeVector(0,0,0); // local point
    G4bool valid = false;               // validity flag
    G4ThreeVector normal = -1*navigator->GetGlobalExitNormal(localPoint, &valid); // idk dont ask too much about G4 geometry
    return normal.unit();
}

G4ThreeVector SiPMSurfaceWLS::GetSpecularAngle(const G4ThreeVector& incident, const G4ThreeVector& normal) {
    return (incident - 2 * incident.dot(normal) * normal).unit();
}

G4ThreeVector SiPMSurfaceWLS::GetDiffuseAngle(const G4ThreeVector& normal) {
    // Generate random angles for isotropic distribution in hemisphere
    G4double phi = G4UniformRand() * 2.0 * CLHEP::pi;  // Azimuthal angle: 0 to 2π
    G4double cosTheta = G4UniformRand();                // cos(polar angle): 0 to 1 for hemisphere
    G4double sinTheta = std::sqrt(1.0 - cosTheta*cosTheta);
    
    // Create direction in local coordinate system (normal = z-axis)
    G4ThreeVector localDir(sinTheta * std::cos(phi),
                          sinTheta * std::sin(phi),
                          cosTheta);
    
    // Create orthogonal basis vectors to transform to global coordinates
    G4ThreeVector u, v;
    if (std::abs(normal.z()) < 0.9) {
        u = G4ThreeVector(0, 0, 1).cross(normal).unit();
    } else {
        u = G4ThreeVector(1, 0, 0).cross(normal).unit();
    }
    v = normal.cross(u);
    // Transform to global coordinates
    G4ThreeVector globalDir = localDir.x() * u + localDir.y() * v + localDir.z() * normal;
    return globalDir.unit();
}

// G4double SiPMSurfaceWLS::ShadowingReduction_transmission(G4double angle) {
    
// }

//==============================================================================
// Fill factor utility functions for shadowing
//==============================================================================
std::pair<G4double, G4double> SiPMSurfaceWLS::QuadraticSolution(G4double a, G4double b, G4double c) {
    G4double discriminant = b*b - 4*a*c;
    if (discriminant < 0) {
        G4cout << "Warning: QuadraticSolution - negative discriminant!" << G4endl;
        return std::make_pair(0.0, 0.0);
    }
    G4double sqrtDisc = std::sqrt(discriminant);
    G4double sol1 = (-b + sqrtDisc) / (2*a);
    G4double sol2 = (-b - sqrtDisc) / (2*a);
    return std::make_pair(sol1, sol2);
}

// This returns the relative size of the microstructure width wrt pitch
G4double SiPMSurfaceWLS::W_from_FF(G4double FF) {
    std::pair<G4double, G4double> solutions = QuadraticSolution(1.0, -1.0, (1.0-FF)/4.0);
    return std::min(solutions.first, solutions.second);
}

G4double SiPMSurfaceWLS::FF_from_W(G4double W) {
    return 4.0*(W*W) - 4.0*W + 1.0;
}

//==============================================================================
// Angular fill factor functions with shadowing
//==============================================================================
G4double SiPMSurfaceWLS::FF_theta_relative_shadow(G4double theta, G4double W, G4double H) { // theta in degrees
    G4double theta_rad = std::abs(theta * CLHEP::deg);  // Convert to radians
    return (1.0 - 2.0*W) * H * std::tan(theta_rad);
}

G4double SiPMSurfaceWLS::FF_theta_relative_transmission(G4double theta, G4double W, G4double H) {
    // W defined as ff/p
    // H defined as h/p
    return FF_from_W(W) - FF_theta_relative_shadow(theta, W, H);
}

G4double SiPMSurfaceWLS::FF_theta_relative_reflection(G4double theta, G4double W, G4double H) {
    return FF_from_W(W) - 2.0*FF_theta_relative_shadow(theta, W, H);
}
