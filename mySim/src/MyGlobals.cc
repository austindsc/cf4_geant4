#include "MyGlobals.hh"

namespace MG{
  static int atownsNumber = 69;
  static int myVerbose = 0;
  
  void SetVerbosity(int a){ myVerbose = a;}
  int GetVerbosity(){ return myVerbose; }
}

namespace DETECTORSETUP{

  // default values in here
  static bool buildWindow = true;
  static bool buildSurface = true; // setting this to false fully changes/breaks the SiPM effective correction
  static bool turnOnQuartzAbs = false;
  static bool doOpticsDiffusion = true; // this to false also will mess up PDE by factor of Rdiffuse (0.17 nominally)

  static bool absorbingAll = false;
  static bool makeVisible = false; // this scales up the small things

  bool GetWindowStatus(){ return buildWindow; }
  void SetWindowStatus(bool a){ buildWindow = a; }

  bool GetSurfaceStatus(){ return buildSurface; }
  void SetSurfaceStatus(bool a){ buildSurface = a; }

  bool GetQuartzABSstatus(){ return turnOnQuartzAbs; }
  void SetQuartzABSstatus(bool a){ turnOnQuartzAbs = a; }

  bool GetDiffusionStatus(){ return doOpticsDiffusion; }
  void SetDiffusionStatus(bool a){ doOpticsDiffusion = a; }

  bool GetVisibleStatus(){ return makeVisible; }
  void SetVisibleStatus(bool a){ makeVisible = a; }

  bool GetAbsorbingStatus(){ return absorbingAll; }
  void SetAbsorbingStatus(bool a){ absorbingAll = a; }

}