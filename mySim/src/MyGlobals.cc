#include "MyGlobals.hh"

namespace MG{
  static int atownsNumber = 69;
  static int myVerbose = 0;
  
  void SetVerbosity(int a){ myVerbose = a;}
  int GetVerbosity(){ return myVerbose; }
}

namespace DETECTORSETUP{
  // default values in here
  static bool buildWindow = false;
  static bool buildSurface = true;
  static bool turnOnQuartzAbs = true;
  static bool makeVisible = false; // scale up the small things

  bool GetWindowStatus(){ return buildWindow; }
  void SetWindowStatus(bool a){ buildWindow = a; }

  bool GetSurfaceStatus(){ return buildSurface; }
  void SetSurfaceStatus(bool a){ buildSurface = a; }

  bool GetQuartzABSstatus(){ return turnOnQuartzAbs; }
  void SetQuartzABSstatus(bool a){ turnOnQuartzAbs = a; }

  bool GetVisibleStatus(){ return makeVisible; }
  void SetVisibleStatus(bool a){ makeVisible = a; }
}