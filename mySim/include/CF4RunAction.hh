//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file CF4RunAction.hh
/// \brief Definition of the CF4RunAction class

#ifndef CF4RunAction_h
#define CF4RunAction_h 1

#include "G4UserRunAction.hh"
#include "G4Accumulable.hh"
#include "globals.hh"
#include "CF4PrimaryGeneratorAction.hh"

class G4Run;
// class CF4PrimaryGeneratorAction;

/// Run action class
///
/// In EndOfRunAction(), it calculates the dose in the selected volume 
/// from the energy deposit accumulated via stepping and event actions.
/// The computed dose is then printed on the screen.

class CF4RunAction : public G4UserRunAction
{
  friend class CF4EventAction;
  friend class CF4PrimaryGeneratorAction;
  public:
    CF4RunAction(CF4PrimaryGeneratorAction * genny);
    virtual ~CF4RunAction();

    // virtual G4Run* GenerateRun();
    virtual void BeginOfRunAction(const G4Run*);
    virtual void   EndOfRunAction(const G4Run*);

  private:
    G4Accumulable<G4double> fEdep;
    G4Accumulable<G4double> fEdep2;

    CF4PrimaryGeneratorAction * fGenerator;
    
    G4double favgEdep;
    G4int    ftotalMCPE;
    G4int    ftotalFired;
};

#endif

