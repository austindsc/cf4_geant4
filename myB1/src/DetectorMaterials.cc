#include "DetectorMaterials.hh"

#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4String.hh"

#include "G4Material.hh"
#include "G4Element.hh"

#include "HelperFunctions.hh"

#include "DetectorMaterials.hh"

// confused about this. but it is necessary.
// this is where we instantiate the globally used singleton pointer.
DetectorMaterials * DetectorMaterials::singleton=nullptr;

DetectorMaterials::DetectorMaterials(){
  
  // default values here
  fDetectorMedium = &mGasCF4;  // default
  fmediumDensity = -1.* g/cm3;  // flagged as not set
  fmediumLY = -1./MeV;         // flagged as not set
  fmediumTemp = -110. * kelvin;      // medium temperature  
  fmediumPressure = -1 * bar;
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
  
  // -------------------- Gas CF4 --------------------//
  G4double cf4Gas_ly = 2500 /MeV;         // rough, taken from python work and slides
  G4double density_cf4Gas = 0.0022*g/cm3; // default value? need to make P and T function to get density
  G4double cf4Gas_temp = 160*kelvin;
  G4double cf4Gas_P = 0.5*bar;
  
  if(*fDetectorMedium == mGasCF4){
    if(fmediumLY > 0) cf4Gas_ly = fmediumLY;
    if(fmediumPressure > 0) cf4Gas_P = fmediumPressure;
    if(fmediumTemp > 0) cf4Gas_temp = fmediumTemp;
    if(fmediumDensity > 0) density_cf4Gas = fmediumDensity;
    else if(fmediumTemp > 0){ // set density based on temperature
      
      
      // do stuff to get the gas density
    }
  }
  
  
  G4Element * elCarbon = new G4Element("Carbon", "C", 6, 12.01 * g/mole);
  G4Element * elFluorine = new G4Element("Fluorine", "F", 9,  19. * g/mole);
  
  mGasCF4 = new G4Material("cf4Gas", density_cf4Gas, 2, kStateGas, cf4Gas_temp, cf4Gas_P);
  mGasCF4->AddElement(elCarbon, 1);
  mGasCF4->AddElement(elFluorine, 4);
    
  #include "Optics/GasCF4.icc"
  
  // -------------------- Liquid CF4 --------------------//
  
  // default parameters baked in
  G4double cf4Liq_ly = 4700. /MeV;    // rough, taken from python work and slides
  G4double density_cf4Liq = 1.6*g/cm3;  // default value? unknown if correct
  G4double cf4Liq_temp = 160*kelvin;
  G4double cf4Liq_P = 0.5*bar;

  G4double a = 10;
  
  if(*fDetectorMedium == mLiquidCF4){
    if(fmediumLY > 0) cf4Liq_ly = fmediumLY;
    if(fmediumPressure > 0) cf4Liq_P = fmediumPressure;
    if(fmediumTemp > 0) cf4Liq_temp = fmediumTemp;
    if(fmediumDensity > 0) density_cf4Liq = fmediumDensity;
    else if(fmediumTemp > 0){// set density based on temperature
      
      // G4cout << fmediumTemp/kelvin << G4endl;
      // G4cout << PH::GetLiquidDensity_eqn(fmediumTemp)/(g/cm3) << G4endl;
      PH::GetDensity_data(fmediumTemp, "data/density_liquidCF4.dat");
      exit(0);
      // do stuff to get the liquid density
    }
  }
  
  mLiquidCF4 = new G4Material("cf4Liq", density_cf4Liq, 2, kStateLiquid, cf4Liq_temp, cf4Liq_P);
  mLiquidCF4->AddElement(elCarbon, 1);
  mLiquidCF4->AddElement(elFluorine, 4);
  #include "Optics/LiquidCF4.icc"
  
  // -------------------- Gaseous Argon --------------------//
  // #include "Optics/testTwoScintillation.icc"


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