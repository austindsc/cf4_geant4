#ifndef SiPMSurfaceWLS_h
#define SiPMSurfaceWLS_h

#include "G4OpBoundaryProcess.hh"
#include "G4OpticalPhoton.hh"
#include "G4VParticleChange.hh"
#include "TwoDLookupTable.hh"
#include "TH2D.h"
#include "SensitiveSIPM.hh"

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
    G4VParticleChange* PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) override;
    
    // Implement required virtual methods from G4OpBoundaryProcess
    void SetInvokeSD(G4bool flag) override { fInvokeSD = flag; }

    void SetupSiPMType(G4String type); // hardcode this for now. In future, this can 
    // take in whatever you like to select SiPM type, and then you can check the SiPM type in the 
    // poststepdoit function to see which SiPM type you are dealing with.
    // better solution is define a SiPM class that holds all the SiPM properties, and then pass a pointer to that class here, 
    // selecting which helper class in PostStepDoIt.
    
    // Voltage setting
    void SetVoltage(G4double voltage) { fVoltage = voltage; }
    G4double GetVoltage() const { return fVoltage; }

    G4double GetFFrelativeDiffuseFraction() const { return fRdiffuseFraction/(1-fFillFactor); } // this is the number to sample from for diffusion when a photon already hits the inactive area

private:
    // Helper methods
    bool EnteringSensitiveDetector(const G4Track& aTrack, const G4Step& aStep);
    G4VParticleChange* DoCustomOpticsAndDetection(const G4Track& aTrack, const G4Step& aStep);

    G4ThreeVector GetSurfaceNormal(const G4Step& aStep);
    G4ThreeVector GetSpecularAngle(const G4ThreeVector& incident, const G4ThreeVector& normal);
    G4ThreeVector GetDiffuseAngle(const G4ThreeVector& normal);

    G4double ShadowingReduction_transmission(G4double angle);
    G4double ShadowingReduction_reflection(G4double angle);

    // Data loading methods
    void LoadOpticsTable(G4bool mandatory = true);
    void LoadInternalPDETable(G4bool mandatory = true);
    
    // Get SensitiveSIPM detector by name
    SensitiveSIPM* GetSiPMDetector(G4String sdName);
    SensitiveSIPM* fSiPMDetector = nullptr;

    // Member variables
    G4bool fInvokeSD;
    
    // 2D Histograms for SiPM optics (extracted from TwoDLookupTable)
    const TH2D* fTransmissionHist;
    const TH2D* fReflectionHist;  
    std::pair<G4double, G4double> fOptics_angleRange; // min/max for wavelength and angle
    std::pair<G4double, G4double> fOptics_wavelengthRange; // min/max for wavelength and angle
    const TH2D* fInternalPDEHist;
    std::pair<G4double, G4double> fInternalPDE_wlenRange; // min/max for wavelength and angle
    std::pair<G4double, G4double> fInternalPDE_voltageRange; // min/max for voltage
    
    // Optical parameters
    G4double fFillFactor;        // FF - geometric fill factor
    G4double fRdiffuseFraction;  // Fraction of reflections that are diffuse vs specular
    G4double fVoltage;           // SiPM bias voltage in volts

    // Fill factor utility functions for shadowing
    G4bool fDoShadowing;
    G4double fResistorHeight; // height of quenching resistor layer in um
    G4double fPixelPitch;     // pixel pitch in um
    G4double fW;              // width/pitch ratio
    G4double fH;              // height/pitch ratio

    std::pair<G4double, G4double> QuadraticSolution(G4double a, G4double b, G4double c);
    G4double W_from_FF(G4double FF);
    G4double FF_from_W(G4double W);
    G4double FF_theta_relative_shadow(G4double theta, G4double W, G4double H);
    G4double FF_theta_relative_transmission(G4double theta, G4double W, G4double H);
    G4double FF_theta_relative_reflection(G4double theta, G4double W, G4double H);
    // see implementation, must scale these by FFo (fFillFactor) to get the proper relative scaling for R, T
    // because the FF call is made first (to check for diffuse reflection)
    
    // Data file paths
    std::string fOpticsDataFile;
    std::string fInternalPDEFile;
    
    // Static instance management
    static SiPMSurfaceWLS* fInstance;
};

#endif
