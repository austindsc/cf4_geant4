
#include "myGlobals.hh"

int atownsNumber = 69;
int myVerbose = 0;

namespace MG{
  void SetVerbosity(int a){ myVerbose = a;}
  int GetVerbosity(){ return myVerbose; }
}