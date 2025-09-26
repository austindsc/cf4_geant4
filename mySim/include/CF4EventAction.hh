/// \file CF4EventAction.hh
/// \brief Definition of the CF4EventAction class

#ifndef CF4EventAction_h
#define CF4EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

class SensitiveSIPM;
class CF4RunAction;
class G4EventManager;

/// Event action class
///

class CF4EventAction : public G4UserEventAction
{
  public:
    CF4EventAction(CF4RunAction* runAction);
    virtual ~CF4EventAction();

    virtual void BeginOfEventAction(const G4Event* event);
    virtual void EndOfEventAction(const G4Event* event);

 void AddEdep(G4double edep){ fEdep += edep; }
 void AddTouchingPE(){ftouchPE++;}
 void AddTransmittedPE(){fmcNPE++;}
 void AddDetPE(){fNPE++;}
    

  private:
    G4EventManager * evMan;
    SensitiveSIPM * fSingleSensitiveSiPM;
    
    CF4RunAction* fRunAction;
    G4double     fEdep;
    G4int        ftouchPE;  // number of photons touching the SiPM
    G4int        fmcNPE;    // mc number of photons entering Silicon (no internal efficiency)
    G4int        fNPE;      // number of photons after internal efficiency
    
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

    
