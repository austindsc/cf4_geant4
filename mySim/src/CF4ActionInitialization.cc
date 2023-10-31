/// \file CF4ActionInitialization.cc
/// \brief Implementation of the CF4ActionInitialization class

#include "CF4ActionInitialization.hh"
#include "CF4PrimaryGeneratorAction.hh"
#include "CF4RunAction.hh"
#include "CF4EventAction.hh"
#include "CF4SteppingAction.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CF4ActionInitialization::CF4ActionInitialization()
 : G4VUserActionInitialization()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CF4ActionInitialization::~CF4ActionInitialization()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CF4ActionInitialization::BuildForMaster() const
{
  // no multithreading
  
  // G4cout << "In here" << G4endl;
  // exit(0);
  // CF4PrimaryGeneratorAction * genny = new CF4PrimaryGeneratorAction();
  // SetUserAction(genny);
  // CF4RunAction* runAction = new CF4RunAction(genny);
  // SetUserAction(runAction);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CF4ActionInitialization::Build() const
{
  CF4PrimaryGeneratorAction * genny = new CF4PrimaryGeneratorAction();
  SetUserAction(genny);

  CF4RunAction* runAction = new CF4RunAction(genny);
  SetUserAction(runAction);
  
  CF4EventAction* eventAction = new CF4EventAction(runAction);
  SetUserAction(eventAction);
  
  SetUserAction(new CF4SteppingAction(eventAction));
}  

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
