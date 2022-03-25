
#include "myGlobals.hh"

namespace MG{
  int atownsNumber = 69;
  int myVerbose = 0;
  
  void SetVerbosity(int a){ myVerbose = a;}
  int GetVerbosity(){ return myVerbose; }
}