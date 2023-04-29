#include "DetectorMaterials.hh"

#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4String.hh"

#include "G4Material.hh"
#include "G4Element.hh"

#include "CLHEP/Units/SystemOfUnits.h"
// #include <algorithm>

#include "HelperFunctions.hh"

#include "DetectorMaterials.hh"

// confused about this. but it is necessary.
// this is where we instantiate the globally used singleton pointer.
DetectorMaterials * DetectorMaterials::singleton=nullptr;

DetectorMaterials::DetectorMaterials(){
  
  // default values here
  fDetectorMedium = &mGasCF4;  // default is gas
  fmediumDensity = -1.* g/cm3;  // flagged as not set
  fmediumTemp = -110. * kelvin;      // medium temperature  
  fmediumPressure = -1 * bar;

  fmediumLY = -1./MeV;  
  fmedium_VisFraction = -1.;       // flagged as not set
  fmediumLY_UV = -1/MeV;          // default is even sharing
  fmediumLY_Vis = -1/MeV;
}

DetectorMaterials *DetectorMaterials::GetInstance(){
  if(singleton==nullptr){ singleton = new DetectorMaterials(); }
  return singleton;
}

DetectorMaterials::~DetectorMaterials(){}

//////////// Done with namespace ////////////

void DetectorMaterials::SetDetectionMedium(G4String med){
  if(med == "gCF4" || med == "GCF4"){
    fDetectorMedium = &mGasCF4;
  } else if (med == "lCF4" || med == "LCF4"){
    fDetectorMedium = &mLiquidCF4;
  } else if (med == "gAr" || med == "GAr"){
    fDetectorMedium = &mGasAr;
  }
}

void DetectorMaterials::BuildMaterials(){
  
  G4cout << "\n-------- Build Materials() -----------\n" << G4endl;

  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();
  
  // -------------------- Chamber Materials --------------------//
  // -- Vacuum -- //
  G4double photonEWorld[2] = {1239.841939*eV/120, 1239.841939*eV/1200}; // set n=1 everywhere
  G4double nWorld[2] = {1.00, 1.00};   
  G4MaterialPropertiesTable *mptWorld = new G4MaterialPropertiesTable();
  mptWorld->AddProperty("RINDEX", photonEWorld, nWorld, 2); //2 is for length of arrays
  
  mVacuum = nist->FindOrBuildMaterial("G4_Galactic");
  mVacuum->SetMaterialPropertiesTable(mptWorld);
  
  // -- Stainless Steel -- //
  mSteel = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
  
  // -- Aluminum -- //
  mAluminum = new G4Material("Aluminum", 13., 26.98*g/mole, 2.700*g/cm3);
  // must do optics
  
  // -- black paper -- //
  // G4MaterialPropertiesTable * mptPaper = new G4MaterialPropertiesTable();
  // mBlackPaper = nist->FindOrBuildMaterial("G4_POLYSTYRENE");
  
  
  // -------------------- Misc Materials --------------------//
  // -- air -- //
  mAir = nist->FindOrBuildMaterial("G4_AIR");
  G4MaterialPropertiesTable *mptAir = new G4MaterialPropertiesTable();
  mptAir->AddProperty("RINDEX", "Air");

  // -- generic plastic -- //
  mGenericPlastic = nist->FindOrBuildMaterial("G4_POLYSTYRENE");
  
  // -- Lead -- //
  mLead = nist->FindOrBuildMaterial("G4_Pb");

  //
  G4Element * elCarbon = new G4Element("Carbon", "C", 6, 12.01 * g/mole);
  G4Element * elFluorine = new G4Element("Fluorine", "F", 9,  19. * g/mole);
  
  // -------------------- Gas CF4 --------------------//
  // default parameters baked in
  G4double cf4Gas_ly = 2500 /MeV;         // rough, taken from python work and slides
  G4double density_cf4Gas = 0.0022*g/cm3; // default value? need to make P and T function to get density
  G4double cf4Gas_temp = 160*kelvin;
  G4double cf4Gas_P = 0.5*bar;
  
  if(*fDetectorMedium == mGasCF4){ // if using gas CF4, set values to those of medium (set via messenger)
    if(fmediumLY > 0) cf4Gas_ly = fmediumLY;
    if(fmediumPressure > 0) cf4Gas_P = fmediumPressure;
    if(fmediumTemp > 0) cf4Gas_temp = fmediumTemp;

    // setting density logic
    if(fmediumTemp > 0) cf4Gas_temp = fmediumTemp;
    if(fmediumDensity > 0) density_cf4Gas = fmediumDensity;
    else if(fmediumTemp > 0 && fmediumPressure > 0 && fmediumDensity < 0){ // set density based on temperature
      // do stuff to get the gas density
      G4ExceptionDescription msg; msg << "Setting density using T, P. CODE NOT READY\n"; 
      G4Exception("DetectorMaterials::BuildMaterials() - Gas CF4 option", "Austin's error message", FatalException, msg);
    }
    else if(fmediumTemp > 0 && fmediumPressure > 0 && fmediumDensity < 0){
      G4ExceptionDescription msg; msg << "User set T, P and density. Only set T,P or Density\n"; 
      G4Exception("DetectorMaterials::BuildMaterials() - Gas CF4 option", "Austin's error message", FatalException, msg);
    }

    mGasCF4 = new G4Material("cf4Gas", density_cf4Gas, 2, kStateGas, cf4Gas_temp, cf4Gas_P);
    mGasCF4->AddElement(elCarbon, 1);
    mGasCF4->AddElement(elFluorine, 4);
    #include "Optics/GasCF4.icc"
  }
  
  
  // -------------------- Liquid CF4 --------------------//

  // default parameters baked in
  G4double cf4Liq_ly = 4700. /MeV;    // rough, taken from python work and slides
  G4double cf4VisFrac = 0.8;
  G4double cf4Liq_lyUV = (1-cf4VisFrac) * cf4Liq_ly;
  G4double cf4Liq_lyVis = cf4VisFrac * cf4Liq_ly;
  G4double density_cf4Liq = 1.6*g/cm3;  // default value? unknown if correct
  G4double cf4Liq_temp = 160*kelvin;
  G4double cf4Liq_P = 0.5*bar;
  
  if(*fDetectorMedium == mLiquidCF4){

    //// LY stuff /////
    if (fmedium_VisFraction >=0. && (fmediumLY_UV >= 0 || fmediumLY_Vis >= 0)){
      // Issue, defined Vis/UV values and Vis fraction.
      G4ExceptionDescription msg; msg << "User defined vis fraction and Vis,UV yields!\n"; 
      G4Exception("DetectorMaterials::BuildMaterials() - Liquid CF4 option", "Austin's error message", FatalException, msg);
    }
    else if(fmedium_VisFraction >=0.) cf4VisFrac = fmedium_VisFraction; // user defined Vis Fraction
    
    if(fmediumLY > 0 && (fmediumLY_UV > 0 || fmediumLY_Vis > 0)){
      // Issue, defined LY and LY UV, Vis values
      G4ExceptionDescription msg; msg << "User set custom mediumLY and mediumVis or UV LY\n"; 
      G4Exception("DetectorMaterials::BuildMaterials() - Liquid CF4 option", "Austin's error message", FatalException, msg);
    }
    else if (fmediumLY >= 0 && (fmediumLY_UV < 0 && fmediumLY_Vis < 0)){
      // set LY, use default vis fraction or user defined (if chosen above in first if statement)
      cf4Liq_ly = fmediumLY;
      cf4Liq_lyUV = 1.-cf4VisFrac;
      cf4Liq_lyVis = cf4VisFrac;
    }
    else if (fmediumLY_UV >= 0 && fmediumLY_Vis >= 0 && fmediumLY < 0){
      // custom sharing (no total LY or user defined fraction)
      fmediumLY = fmediumLY_UV + fmediumLY_Vis;
      cf4Liq_ly = fmediumLY;
      cf4Liq_lyUV = fmediumLY_UV; 
      cf4Liq_lyVis = fmediumLY_Vis;
    }
    else { // something else
      G4ExceptionDescription msg; msg << "Using all default LY stuff\n"; 
      G4Exception("DetectorMaterials::BuildMaterials() - Liquid CF4 option", "Austin's error message", JustWarning, msg);
    }// done with light yields


    //// Density Logic
    if(fmediumTemp > 0) cf4Liq_temp = fmediumTemp; // do these
    if(fmediumDensity > 0) density_cf4Liq = fmediumDensity;
    else if(fmediumTemp > 0 && fmediumPressure > 0 && fmediumDensity < 0){ // set density based on temperature/pressure
      G4ExceptionDescription msg; msg << "Setting density using T, P. CODE NOT READY\n"; 
      G4Exception("DetectorMaterials::BuildMaterials() - Liquid CF4 option", "Austin's error message", FatalException, msg);
      
      // do stuff to get the liquid density
      PH::GetDensity_data(fmediumTemp, "data/density_liquidCF4.dat");
    }

    mLiquidCF4 = new G4Material("cf4Liq", density_cf4Liq, 2, kStateLiquid, cf4Liq_temp, cf4Liq_P);
    mLiquidCF4->AddElement(elCarbon, 1);
    mLiquidCF4->AddElement(elFluorine, 4);
    #include "Optics/testTwoScintillation.icc" // all the physics is in here!
    // #include "Optics/LiquidCF4.icc"
  }
  
  
  // -------------------- GAr --------------------//
  // 


  // -------------------- SiPM Stuff --------------------//
  // -- Silicon -- //
  G4double photonE_Si[2] = {1239.841939*eV/140, 1239.841939*eV/1200};
  G4double nSi[2] = {1.20, 1.20};   
  G4MaterialPropertiesTable *mptSi = new G4MaterialPropertiesTable();
  mptSi->AddProperty("RINDEX", photonE_Si, nSi, 2); //2 is for length of arrays
  mSilicon = nist->FindOrBuildMaterial("G4_Si");
  mSilicon->SetMaterialPropertiesTable(mptSi);
  
  // -- Ceramic --//
  //ceramic(Medical Physics, May 2000), from old BrachyMaterial class
  mCeramic = new G4Material("Ceramic", 2.88*g/cm3, 2, kStateSolid);
  mCeramic->AddElement(nist->FindOrBuildElement("Al"),2);
  mCeramic->AddElement(nist->FindOrBuildElement("O"),3);

  // -------------------- Smoke Detector --------------------//
  mSilver = nist->FindOrBuildMaterial("G4_Ag");
  mPalladium = nist->FindOrBuildMaterial("G4_Pd");
  mGold = nist->FindOrBuildMaterial("G4_Au");

  // -- active matrix -- //
  G4double goldFrac = 0.5;
  G4Element * elAm = new G4Element("Americium", "Am", 95, 243 * g/mole);
  G4Element * elO = new G4Element("Oxygen", "O", 8,  16. * g/mole);
  G4Material * mAmOxide = new G4Material("mAmOxide", 11.68 * g/cm3, 2);
  mAmOxide->AddElement(elAm, 1);
  mAmOxide->AddElement(elO, 2);
  
  G4double sourceDensity = mGold->GetDensity() * goldFrac + mAmOxide->GetDensity()*(1-goldFrac);
  mAmActiveMatrix = new G4Material("mSource", sourceDensity, 2);
  mAmActiveMatrix->AddMaterial(mGold, goldFrac);
  mAmActiveMatrix->AddMaterial(mAmOxide, 1-goldFrac);
  
  // -------------------- More Stuff --------------------//
  return;
}











// end