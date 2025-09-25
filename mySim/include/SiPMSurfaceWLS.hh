#ifndef SiPMSurfaceWLS_h
#define SiPMSurfaceWLS_h

#include "G4OpBoundaryProcess.hh"
#include "G4OpticalPhoton.hh"
#include "G4VParticleChange.hh"

class SiPMSurfaceWLS : public G4OpBoundaryProcess
{
public:
    SiPMSurfaceWLS(const G4String& processName = "SiPMSurfaceWLS");
    ~SiPMSurfaceWLS();
    
    // Singleton access
    static SiPMSurfaceWLS* GetInstance() {
        if (!fInstance) {
            fInstance = new SiPMSurfaceWLS();
        }
        return fInstance;
    }
    
    // Override PostStepDoIt to add custom SiPM logic
    virtual G4VParticleChange* PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) override;
    
    // Implement required virtual methods from G4OpBoundaryProcess
    virtual void SetInvokeSD(G4bool flag) override { fInvokeSD = flag; }

private:
    // Helper methods
    bool EnteringSensitiveDetector(const G4Track& aTrack, const G4Step& aStep);
    G4VParticleChange* ApplyCustomReflection(const G4Track& aTrack, const G4Step& aStep);

    G4ThreeVector GetSurfaceNormal(const G4Step& aStep);
    G4ThreeVector GetSpecularAngle(const G4ThreeVector& incident, const G4ThreeVector& normal);
    G4ThreeVector GetDiffuseAngle(const G4ThreeVector& normal);
    
    // Member variables
    G4bool fInvokeSD;
    
    // Static instance management
    static SiPMSurfaceWLS* fInstance;
};

#endif
