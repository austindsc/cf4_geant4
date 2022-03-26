#ifndef HELPEROPTICAL_H
#define HELPEROPTICAL_H

#include "G4PhysicsOrderedFreeVector.hh"

namespace OH{
  
  bool LoadTxt_OrderedVector(std::string path, G4PhysicsOrderedFreeVector * vec, int verbose = 0, double scaleY = 1);
  
  void testOH();
  
  // namespace SOH{
  //   void testSecondOH();
  // }
}

#endif