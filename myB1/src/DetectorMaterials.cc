#include "DetectorMaterials.hh"

#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4String.hh"

#include "G4Material.hh"
#include "G4Element.hh"

#include "HelperOptical.hh"

#include "DetectorMaterials.hh"

// confused about this. but it is necessary.
// this is where we instantiate the globally used singleton pointer.
DetectorMaterials * DetectorMaterials::singleton=nullptr;

DetectorMaterials::DetectorMaterials(){
  
  // default values here
  fDetectorMedium = &mGasCF4;  // default
  fmediumDensity = -1.* g/cm3;  // flagged as not set
  fmediumLY = -1./MeV;         // flagged as not set
  fmediumTemp = 110. * kelvin;      // medium temperature
  fmediumPressure = 1 * bar;
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
  // -- vacuum -- //
  G4double photonEWorld[2] = {1239.841939*eV/120, 1239.841939*eV/1200}; // set n=1 everywhere
  G4double nWorld[2] = {1.00, 1.00};   
  G4MaterialPropertiesTable *mptWorld = new G4MaterialPropertiesTable();
  mptWorld->AddProperty("RINDEX", photonEWorld, nWorld, 2); //2 is for length of arrays
  
  mVacuum = nist->FindOrBuildMaterial("G4_Galactic");
  mVacuum->SetMaterialPropertiesTable(mptWorld);
  
  // -------------------- Gas CF4 --------------------//
  G4double density_cf4Gas = 0.0012*g/cm3; // default value? need to make P and T function to get density
  G4double cf4Gas_ly = 2500 /MeV;         // rough, taken from python work and slides
  if(fmediumDensity > 0 && *fDetectorMedium == mGasCF4) density_cf4Gas = fmediumDensity;
  if(fmediumLY > 0 && *fDetectorMedium == mGasCF4) cf4Gas_ly = fmediumLY;
  
  G4Element * elCarbon = new G4Element("Carbon", "C", 6, 12.01 * g/mole);
  G4Element * elFluorine = new G4Element("Fluorine", "F", 9,  19. * g/mole);
  
  mGasCF4 = new G4Material("cf4Gas", density_cf4Gas, 2, kStateGas, fmediumTemp, fmediumPressure);
  mGasCF4->AddElement(elCarbon, 1);
  mGasCF4->AddElement(elFluorine, 4);
    
  #include "Optics/GasCF4.icc"
  
  // -------------------- Liquid CF4 --------------------//
  
  G4double density_cf4Liq = 1.6*g/cm3;  // default value? unknown if correct
  G4double cf4Liq_ly = 4700 /MeV;       // rough, taken from python work and slides
  
  if(fmediumDensity > 0 && *fDetectorMedium == mLiquidCF4) density_cf4Liq = fmediumDensity;
  if(fmediumLY > 0 && *fDetectorMedium == mLiquidCF4) cf4Liq_ly = fmediumLY;
  mLiquidCF4 = new G4Material("cf4Liq", density_cf4Liq, 2, kStateLiquid, fmediumTemp, fmediumPressure);
  mLiquidCF4->AddElement(elCarbon, 1);
  mLiquidCF4->AddElement(elFluorine, 4);
  
  #include "Optics/LiquidCF4.icc"
  
  
  // -------------------- Gaseous Argon --------------------//

  // -------------------- Silicon --------------------//
  
  G4double photonE_Si[2] = {1239.841939*eV/140, 1239.841939*eV/1200};
  G4double nSi[2] = {1.20, 1.20};   
  G4MaterialPropertiesTable *mptSi = new G4MaterialPropertiesTable();
  mptSi->AddProperty("RINDEX", photonE_Si, nSi, 2); //2 is for length of arrays
  mSilicon = nist->FindOrBuildMaterial("G4_Si");
  mSilicon->SetMaterialPropertiesTable(mptSi);

  // -------------------- Misc Materials --------------------//
  // -- Stainless Steel -- //
  mSteel = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
  
  // -- Lead -- //
  mLead = nist->FindOrBuildMaterial("G4_Pb");

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