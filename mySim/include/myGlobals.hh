#ifndef MYGLOBALS_H
#define MYGLOBALS_H

#include "globals.hh"

namespace MG{
// this is just a global header to control verbosity across any files I want.
// simply add #include "myGlobals.hh" to any file using them.
extern int atownsNumber; // testing
extern int myVerbose;

// i could define all of my ntuple integer stuff here, but don't need it
// extern

extern G4String whatSource;

// numbers aren't private, so getters/setters aren't needed
// however its nice to keep the illusion of good code going (lol)
void SetVerbosity(int a);
int GetVerbosity();
}

#endif