#ifndef HELPEROPTICAL_H
#define HELPEROPTICAL_H

#include "G4PhysicsOrderedFreeVector.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include <fstream>
#include <vector>
#include <iostream>


namespace OH{ //optical helper
  
  bool LoadTxt_OrderedVector(std::string path, G4PhysicsFreeVector * vec, int verbose = 0, G4double scaleX = 1.0, G4double scaleY = 1.0);
  
  bool ReadAndFillArray(const std::string& filename, std::vector<G4double>& dataArray1, std::vector<G4double>& dataArray2, int verbose = 0, G4double scale1stValue = 1., G4double scale2ndValue = 1., bool doFlip_wls2E = false);

  void testOH();
  
  // namespace SOH{
  //   void testSecondOH();
  // }
}

namespace PH{ //physical helper
  
  G4double GetLiquidDensity_eqn(G4double temp); //return in g/cm3
  // https://pubs.acs.org/doi/10.1021/je60025a016
  
  G4double GetDensity_data(G4double temp, G4String path); //return in g/cm3
  // from NIST: https://webbook.nist.gov/cgi/cbook.cgi?ID=C75730&Mask=B
  
  // #ifdef USE_REFPROP // not needed, give two definitions
  G4double GetREFPROP_density(std::string materialName, double temp, double pressure);
  // pass pressure in kPa
  // return in g/cm3, using REFPROP
  // #endif

}

#endif