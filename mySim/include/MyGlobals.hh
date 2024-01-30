#ifndef MYGLOBALS_H
#define MYGLOBALS_H

#include "globals.hh"

// this is just a global header to control verbosity across any files I want.
// simply add #include "MyGlobals.hh" to any file using them. only use the getters

// want to avoid doing a singleton class, so i do it a different way
// can't do private in a namespace, so just give getters and setters here.
// then define the variables as static in the .cc file (only seen from .cc file)

namespace MG{

  // things defined in the .cc file
  // extern int atownsNumber; // testing
  // extern int myVerbose;

  void SetVerbosity(int a);
  int GetVerbosity();
}

namespace DETECTORSETUP{

  // things defined in the .cc file
  // extern bool buildWindow;
  // extern bool buildSurface;
  // extern bool turnOnQuartzAbs;
  // extern bool makeVisible;

  bool GetWindowStatus();
  void SetWindowStatus(bool a);

  bool GetSurfaceStatus();
  void SetSurfaceStatus(bool a);

  bool GetQuartzABSstatus();
  void SetQuartzABSstatus(bool a);

  bool GetDiffusionStatus();
  void SetDiffusionStatus(bool a);

  bool GetVisibleStatus();
  void SetVisibleStatus(bool a);

}

// ---- not doing singleton setup although it is sexier ----//
// class MyGlobals{
// public:
//   static MyGlobals& getInstance(){
//     static MyGlobals instance; // Guaranteed to be destroyed, instantiated on first use.
//     return instance;
//   }

//   int atownsNumber = 69;
//   int myVerbose = 0;

//   private:
//     MyGlobals() {}
//     MyGlobals(MyGlobals const&);     // Don't Implement
//     void operator=(MyGlobals const&); // Don't implement
// };

// class DetectorSetup{
// public:
//   static DetectorSetup& getInstance(){
//     static DetectorSetup instance; // Guaranteed to be destroyed, instantiated on first use.
//     return instance;
//   }

//   bool buildWindow;
//   bool buildSurface;
//   bool turnOnQuartzAbs;

//   private:
//     Global() {}
//     Global(Global const&);     // Don't Implement
//     void operator=(Global const&); // Don't implement
// };

#endif