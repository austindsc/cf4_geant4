/// \file CF4ActionInitialization.hh
/// \brief Definition of the CF4ActionInitialization class

#ifndef CF4ActionInitialization_h
#define CF4ActionInitialization_h 1

#include "G4VUserActionInitialization.hh"
#include "CF4RunAction.hh"

/// Action initialization class.

class CF4ActionInitialization : public G4VUserActionInitialization
{
  public:
    CF4ActionInitialization();
    virtual ~CF4ActionInitialization();

    virtual void BuildForMaster() const;
    virtual void Build() const;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

    
