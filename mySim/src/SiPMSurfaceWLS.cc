#include "SiPMSurfaceWLS.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VSensitiveDetector.hh"
#include "G4StepPoint.hh"
#include "G4TransportationManager.hh"
#include "G4UniformRand.hh"
#include "CLHEP/Units/PhysicalConstants.h"
#include <cmath>

// Static member initialization
SiPMSurfaceWLS* SiPMSurfaceWLS::fInstance = nullptr;

//==============================================================================
// Constructor - inherits from G4OpBoundaryProcess
//==============================================================================
SiPMSurfaceWLS::SiPMSurfaceWLS(const G4String& processName)
    : G4OpBoundaryProcess(processName), fInvokeSD(true) {
    
    fInstance = this;
    G4cout << "SiPMSurfaceWLS: Initialized as enhanced boundary process" << G4endl;
}

SiPMSurfaceWLS::~SiPMSurfaceWLS() {
    if (fInstance == this) {
        fInstance = nullptr;
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
    // Check if we're entering a sensitive detector (SiPM)
    if (EnteringSensitiveDetector(aTrack, aStep)) {
      // this is us!
      G4cout << "SiPMSurfaceWLS: At SiPM boundary - applying custom optics" << G4endl;
      return ApplyCustomReflection(aTrack, aStep);
    }
    
    // Not at SiPM boundary - use standard G4OpBoundaryProcess behavior
    return G4OpBoundaryProcess::PostStepDoIt(aTrack, aStep);
}

//==============================================================================
// Check if entering a sensitive detector volume
bool SiPMSurfaceWLS::EnteringSensitiveDetector(const G4Track& aTrack, const G4Step& aStep) {
    G4StepPoint* postStepPoint = aStep.GetPostStepPoint();
    G4VPhysicalVolume* nextVolume = postStepPoint->GetPhysicalVolume();

    G4bool boundaryStatus = (aStep.GetPreStepPoint()->GetStepStatus() == fGeomBoundary);
    
    if (nextVolume && boundaryStatus == fGeomBoundary) {
        G4LogicalVolume* logicalVol = nextVolume->GetLogicalVolume();
        if (logicalVol && logicalVol->GetSensitiveDetector()) {
            return true;
        }
    }
    return false;
}

//==============================================================================
// doing some math
//==============================================================================
G4ThreeVector SiPMSurfaceWLS::GetSurfaceNormal(const G4Step& aStep) {
    G4Navigator* navigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
    
    // GetGlobalExitNormal typically needs these parameters:
    G4ThreeVector localPoint = G4ThreeVector(0,0,0); // local point
    G4bool* valid = new G4bool(false);               // validity flag
    G4ThreeVector normal = -1*navigator->GetGlobalExitNormal(localPoint, valid); // idk dont ask too much about G4 geometry
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

//==============================================================================
// Apply custom reflection at SiPM surface
//==============================================================================
G4VParticleChange* SiPMSurfaceWLS::ApplyCustomReflection(const G4Track& aTrack, const G4Step& aStep) {
    aParticleChange.Initialize(aTrack);
    
    // Get current position and direction
    G4ThreeVector currentPos = aTrack.GetPosition();
    G4ThreeVector currentDir = aTrack.GetMomentumDirection().unit();

    // do some math
    G4ThreeVector normal = GetSurfaceNormal(aStep);
    G4ThreeVector specularVec = GetSpecularAngle(currentDir, normal);
    G4ThreeVector diffuseVec = GetDiffuseAngle(normal);

    specularVec = specularVec.unit();
    aParticleChange.ProposeMomentumDirection(specularVec);
    
    G4cout << "  New direction: " << specularVec << G4endl;
    
    return &aParticleChange;
}
