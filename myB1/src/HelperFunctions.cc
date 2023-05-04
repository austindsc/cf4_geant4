
#include "globals.hh"
#include "HelperFunctions.hh"
#include "myGlobals.hh"

// REFPROP stuff //
#ifdef USE_REFPROP // from CMAKE

#define REFPROP_IMPLEMENTATION
#define REFPROP_FUNCTION_MODIFIER
#include "REFPROP_lib.h"
#undef REFPROP_IMPLEMENTATION
#undef REFPROP_FUNCTION_MODIFIER

#endif

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
  
  G4double GetDensity_data(G4double temp, G4String path){ // returns in g/cm3
    G4PhysicsFreeVector * tandrho = new G4PhysicsFreeVector(false); // spline OFF
    OH::LoadTxt_OrderedVector(path, tandrho, 1, kelvin, 1e-3*g/cm3); // convert to g/cm3 with last argument
    
    if(MG::GetVerbosity() > 0) tandrho->DumpValues();
    G4cout << "(In GetDensity_data), at temp = " << temp << " , Rho = " << tandrho->Value(temp)/(g/cm3) << " g/cm3" << G4endl;
    return tandrho->Value(temp/kelvin);
  }

  #ifdef USE_REFPROP // using REFPROP
  G4double GetREFPROP_density(std::string materialName, double ftemp, double fpressure){
    // pass pressure in bar, temp in K
    // set paths
    std::string path = std::getenv("REFPROPDLLPATH");
    std::string DLL_name = "librefprop.dylib";
    std::string err;

    // Load the shared library
    bool loaded_REFPROP = load_REFPROP(err, path, DLL_name);
    printf("Loaded refprop (in HelperFunctions.cc): %s @ address %zu\n", loaded_REFPROP ? "true" : "false", REFPROP_address());
    if (!loaded_REFPROP) G4Exception("HelperFunctions::GetREFPROP_density", "Austin error message", FatalException, "Couldn't load REFPROP");
    SETPATHdll(const_cast<char*>(path.c_str()), 400); // idk, this was in example code

    // setup what I'm asking for (partially)
    int ierr = 0;
    int nc = 1; // pure fluid
    char hfld[10000] = "water"; // what fluid!
    char herr[255];
    char hhmx[255] = "HMX.BNC"; // idk
    char href[4] = "DEF"; // idk
    SETUPdll(nc,hfld,hhmx,href,ierr,herr,10000,255,3,255);

    // if call to setupdll works
    if (ierr > 0) printf("This ierr: %d herr: %s\n", ierr, herr);{
        int ierr = 0;
        char herr[255];
        double z[20] = {1.0}, x[20] = {1.0}, y[20] = {1.0}, d = -1, dl = -1, dv = -1, h = -1, s = -1, u = -1, cp = -1, cv = -1, q = -1, w = -1;
        double T= 300, p = 101.325;

        TPFLSHdll(T, p, z, d, dl, dv, x, y, h,s,u,cp,cv,w,q,ierr,herr,255);
        // TPFLSHdll(T, p, z, d, dl, dv, x);
        if (ierr > 0) printf("This ierr: %d herr: %s\n", ierr, herr);
        printf("This d: %g mol/L\n", d);
        {
            double T = ftemp;
            double z[20] = { 0.20209999999999995, 0.7979};
            int kph = 1;
            double P, rhol, rhov;
            double x[20], y[20];
            int ierr; char herr[255];
            SATTdll(T, z, kph, P, rhol, rhov, x, y, ierr, herr, 255);
            printf("This p: %g mol/L\n", p);

            exit(0);
        }
    }

    // unload refprop
    bool unloaded_REFPROP = unload_REFPROP(err);
    return 1;
  }
  #else 
  G4double GetREFPROP_density(std::string materialName, double temp, double density){
    G4ExceptionDescription msg; msg << "Asked for density via T, P, but REFPROP not built";
    G4Exception("DetectorMaterials::BuildMaterials() - HelperFunctions", "Austin's error message", FatalException, msg);
  }
  #endif // end of function 


}