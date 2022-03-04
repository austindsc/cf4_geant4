#include "DetectorMaterials.hh"

#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4String.hh"

#include "G4Material.hh"
#include "G4Element.hh"

DetectorMaterials::DetectorMaterials(){
  
  // default values here
  fDetectorMedium = mGasCF4;  // default
  fmediumDensity = -1 g/cm3;  // flagged as not set
  fmediumLY = -1/MeV;         // flagged as not set
}

DetectorMaterials::~DetectorMaterials(){
}

void DetectorMaterials::SetDetectionMedium(G4String s){
  if(s == "gCF4" || s == "GCF4"){
    G4cout << "YOOOO we here" << G4endl;
    // exit(0);
    fDetectorMedium = mGasCF4;
  } else if (s == "lCF4" || s == "LCF4"){
    fDetectorMedium = mLiquidCF4;
  } else if (s == "gAr" || s == "GAr"){
    fDetectorMedium = mGasAr;
  }
}

void DetectorMaterials::ConstructMaterials(){
  
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();
  
  // -------------------- Chamber Materials --------------------//
  // -- vacuum -- //
  G4double photonEWorld[2] = {1239.841939*eV/140, 1239.841939*eV/1200};
  G4double nWorld[2] = {1.00, 1.00};   
  G4MaterialPropertiesTable *mptWorld = new G4MaterialPropertiesTable();
  mptWorld->AddProperty("RINDEX", photonEWorld, nWorld, 2); //2 is for length of arrays
  
  mVacuum = nist->FindOrBuildMaterial("G4_Galactic");
  mVacuum->SetMaterialPropertiesTable(mptWorld);
  
  // -- Stainless Steel -- //
  mSteel = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
  // -- Lead -- //
  mLead = nist->FindOrBuildMaterial("G4_Pb");
  
  // -------------------- Silicon --------------------//
  
  G4double photonE_Si[2] = {1239.841939*eV/140, 1239.841939*eV/1200};
  G4double nSi[2] = {1.20, 1.20};   
  G4MaterialPropertiesTable *mptSi = new G4MaterialPropertiesTable();
  mptSi->AddProperty("RINDEX", photonE_Si, nSi, 2); //2 is for length of arrays
  mSilicon = nist->FindOrBuildMaterial("G4_Si");
  mSilicon->SetMaterialPropertiesTable(mptSi);
  
  // -------------------- Gas CF4 --------------------//
  
  G4double density_cf4Gas = 0.0014*g/cm3; // default value
  G4double cf4Gas_ly = 1000 /MeV;
  if(fmediumDensity > 0 && fDetectorMedium == mGasCF4) density_cf4Gas = fmediumDensity;
  if(fmediumLY > 0 && fDetectorMedium == mGasCF4) cf4Gas_ly = fmediumLY;

  G4Element * elCarbon = new G4Element("Carbon", "C", 6, 12.01 * g/mole);
  G4Element * elFluorine = new G4Element("Fluorine", "F", 9,  19. * g/mole);
  
  mGasCF4 = new G4Material("cf4Gas", density_cf4Gas, 2);
  mGasCF4->AddElement(elCarbon, 1);
  mGasCF4->AddElement(elFluorine, 4);
  
  G4MaterialPropertiesTable *mpt_gascf4 = new G4MaterialPropertiesTable();
  
  // refractive indexes taken from
  // https://doi.org/10.1063/1.1695909
  G4double photonE[2] = {1239.841939*eV/140, 1239.841939*eV/1200};
  G4double cf4n[2] = {1.1, 1.1};
  mpt_gascf4->AddProperty("RINDEX", photonE, cf4n, 2);    //2 is for length of arrays
  
  // add scintillation here
  G4double scint1_wl[1] = { 3. * eV };
  G4double scintFast[1] = { 1. };
  G4double *scintSlow = scintFast;
  
  mpt_gascf4->AddProperty("FASTCOMPONENT", scint1_wl, scintFast, 2);
  mpt_gascf4->AddProperty("SLOWCOMPONENT", scint1_wl, scintSlow, 2);
  
  mpt_gascf4->AddConstProperty("SCINTILLATIONYIELD", cf4Gas_ly);
  // mpt_gascf4->AddConstProperty("SCINTILLATIONYIELD", 0.0 /MeV);
  mpt_gascf4->AddConstProperty("RESOLUTIONSCALE", 2); 
  mpt_gascf4->AddConstProperty("FASTTIMECONSTANT", 5*ns); 
  mpt_gascf4->AddConstProperty("SLOWTIMECONSTANT", 20*ns); 
  mpt_gascf4->AddConstProperty("YIELDRATIO", 1.); 
  
  mGasCF4->GetIonisation()->SetBirksConstant(0 * mm/MeV);
  mGasCF4->SetMaterialPropertiesTable(mpt_gascf4);
  
  // -------------------- Liquid CF4 --------------------//
  mLiquidCF4 = mGasCF4; //temporary
  
  G4double density_cf4Liq = 1.4*g/cm3; // default value
  G4double cf4Liq_ly = 500 /MeV;
  
  if(fmediumDensity > 0 && fDetectorMedium == mLiquidCF4) density_cf4Liq = fmediumDensity;
  if(fmediumLY > 0 && fDetectorMedium == mLiquidCF4) cf4Liq_ly = fmediumLY;
  
  // make CF4
  
  // -------------------- Gaseous Argon --------------------//

  
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