/// \file CF4EventAction.hh
/// \brief Definition of the CF4EventAction class

#ifndef CF4EventAction_h
#define CF4EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

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
 void AddTruthPE(){fmcNPE++;}
 void AddDetPE(){fNPE++;}
    

  private:
    G4EventManager * evMan;
    
    CF4RunAction* fRunAction;
    G4double     fEdep;
    G4int        fmcNPE; // mc number of photons hitting detector (no efficiencies)
    G4int        fNPE;  // after efficiencies
    
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

    
