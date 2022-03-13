#ifndef HELPEROPTICAL_H
#define HELPEROPTICAL_H

class G4PhysicsOrderedFreeVector;

namespace OH{
  
  bool LoadTxt_OrderedVector(std::string path, G4PhysicsOrderedFreeVector * vec, int verbose = 0, double scaleY = 1);
  
  void testOH();
  
  // namespace SOH{
  //   void testSecondOH();
  // }
}

#endif