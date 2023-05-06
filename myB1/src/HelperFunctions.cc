
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
    
    // this is density from saturation curve

    if(MG::GetVerbosity() > 0) tandrho->DumpValues();
    G4cout << "(In GetDensity_data), at temp = " << temp << " , Rho = " << tandrho->Value(temp)/(g/cm3) << " g/cm3" << G4endl;
    return tandrho->Value(temp/kelvin);
  }

  #ifdef USE_REFPROP // using REFPROP
  G4double GetREFPROP_density(std::string materialName, double ftemp, double fpressure){
    // see REFPROP-headers main.cpp example file for full code where this was taken from
    // and REFPROP documentation for function explanation
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
    int nc = 1; // pure fluid (number of components)
    char hfld[10000];
    std::strcpy(hfld,materialName.c_str()); // what fluid! (what files to load)

    // exit(0);
    char hhmx[255] = "HMX.BNC"; // name of file with mixture coefficient?????
    char href[4] = "DEF";       // something about thermo calc
    int ierr = 0;               // track errors
    char herr[255];             // error message
    SETUPdll(nc,hfld,hhmx,href,ierr,herr,10000,255,3,255); // other args are some default stuff

    // if call to setupdll works
    if (ierr > 0) printf("This ierr: %d herr: %s\n", ierr, herr);{

        // inputs
        double T= ftemp;
        double p = fpressure; // needed in kPa
        double z[20] = {1.0}; // pure fluid

        // output i care about
        double density = -1.; // output in mol/L

        // outputs i don't care about
        double x[20] = {1.0}, y[20] = {1.0}, dl = -1, dv = -1, h = -1, s = -1, u = -1, cp = -1, cv = -1, q = -1, w = -1;
        
        TPFLSHdll(T, p, z, density, dl, dv, x, y, h,s,u,cp,cv,w,q,ierr,herr,255);
        if (ierr > 0) {
          printf("This ierr: %d herr: %s\n", ierr, herr);
          G4Exception("DetectorMaterials::BuildMaterials() - HelperFunctions::GetREFPROP_density", "Austin's error message", 
          FatalException, "something wrong with REFPROP density CALC");
        }

        // Convert the density from mol/L to g/cm3
        double MW = 0; // Molar mass of the substance (in g/mol)
        int fortranIsStupid = 1;
        WMOLIdll(fortranIsStupid, MW);

        G4double rho_gcm3 = density * MW / 1000. * g/cm3; // mol/L x g/mol x 1L/1000cm3
        G4cout << "calculated density of " << materialName << " : " << rho_gcm3/(g/cm3) << "g/cm3" << G4endl;
        return rho_gcm3;
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