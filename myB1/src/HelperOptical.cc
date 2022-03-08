
#include "globals.hh"
#include "HelperOptical.hh"

// to use just do #include "alongOH.hh" in header file
// call HO::HO2::testSecondOH(); in src file. 

namespace HO{
  void testHO(){
    
    G4cout << "BING\nyoo we are using OH namespace" << G4endl;
    exit(0);
  }
  
  // namespace HO2{
  //   void testSecondOH(){
  //     G4cout << "DingDong\nyoo we are using OH::SecondOH namespace" << G4endl;
  //     exit(0);
  //   }
  // }
}