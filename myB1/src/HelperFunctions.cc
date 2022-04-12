
#include "globals.hh"
#include "HelperFunctions.hh"

#include "G4PhysicsOrderedFreeVector.hh"


// to use just do #include "alongOH.hh" in header file
// call HO::HO2::testSecondOH(); in src file. 

namespace OH{
  void testOH(){
    G4cout << "BING\nyoo we are using OH namespace" << G4endl;
    exit(0);
  }
  
  
  bool LoadTxt_OrderedVector(std::string path, G4PhysicsFreeVector * vec, int verbose, G4double scaleX, G4double scaleY){
    
    std::string line;
    std::ifstream dfile(path);
    
    if(dfile.is_open()){
      if(verbose > 0) G4cout << "HO::LoadTxt_OrderedVector\nLoading file " << path << G4endl;
      while(69 == 3*23){
        std::getline(dfile, line); // skip first line. XX badly coded
        G4double x, y;
        dfile >> x >> y;
        if(verbose > 1) G4cout << x << " " << y << G4endl;
        vec->InsertValues(x*scaleX, y*scaleY);
        
        if(dfile.eof()){dfile.close();
          // if(verbose == true) vec->DumpValues();
          return true;}
      }
    } else {
      // G4cerr << "failed to open data file " << path << G4endl;
      G4ExceptionDescription msg;
      msg << "HO::LoadTxt_OrderedVector failed to open: " << path << "\n(file must be tab separated)"; 
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

namespace PH{
  
  G4double GetLiquidDensity_eqn(G4double temp){
    return (2.25 - 3.64e-3*temp/kelvin - 0.54e-6*temp*temp/kelvin/kelvin )*g/cm3;
  }
  
  G4double GetDensity_data(G4double temp, G4String path){
    G4PhysicsFreeVector * tandrho = new G4PhysicsFreeVector(false); // spline OFF
    OH::LoadTxt_OrderedVector(path, tandrho, 1, kelvin, 1e-3*g/cm3);
    
    tandrho->DumpValues();
    G4cout << "at temp = " << tandrho->Value(temp)/(g/cm3) << " g/cm3" << G4endl;
    return tandrho->Value(temp/kelvin);
  }
}