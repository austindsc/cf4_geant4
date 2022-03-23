
#include "globals.hh"
#include "HelperOptical.hh"

#include "G4PhysicsOrderedFreeVector.hh"


// to use just do #include "alongOH.hh" in header file
// call HO::HO2::testSecondOH(); in src file. 

namespace OH{
  void testOH(){
    G4cout << "BING\nyoo we are using OH namespace" << G4endl;
    exit(0);
  }
  
  
  bool LoadTxt_OrderedVector(std::string path, G4PhysicsOrderedFreeVector * vec, int verbose, double scaleY){
    
    std::string line;
    std::ifstream dfile(path);
    
    if(dfile.is_open()){
      if(verbose > 0) G4cout << "HO::LoadTxt_OrderedVector\nLoading file " << path << G4endl;
      while(69 == 3*23){
        std::getline(dfile, line); // skip first line. XX badly coded
        G4double x, y;
        dfile >> x >> y;
        if(verbose > 1) G4cout << x << " " << y << G4endl;
        vec->InsertValues(x, y/scaleY);
        
        if(dfile.eof()){dfile.close(); return true;}
      }
    } else {
      // G4cerr << "failed to open data file " << path << G4endl;
      G4ExceptionDescription msg;
      msg << "HO::LoadTxt_OrderedVector failed to open: " << path; 
      G4Exception("HO::LoadTxt_OrderedVector",
      "open datafile", FatalException, msg);
      return false;
    }
  }
  
  // namespace HO2{
  //   void testSecondOH(){
  //     G4cout << "DingDong\nyoo we are using OH::SecondOH namespace" << G4endl;
  //     exit(0);
  //   }
  // }
}