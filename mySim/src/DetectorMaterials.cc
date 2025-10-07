#include "DetectorMaterials.hh"

#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4String.hh"

#include "G4Material.hh"
#include "G4Element.hh"
#include "G4OpticalSurface.hh"

#include "globals.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include <CLHEP/Units/SystemOfUnits.h>


#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"

// #include <algorithm>

#include "HelperFunctions.hh"
#include "MyGlobals.hh"

#include "DetectorMaterials.hh"

// confused about this. but it is necessary.
// this is where we instantiate the globally used singleton pointer.
DetectorMaterials * DetectorMaterials::singleton=nullptr;

DetectorMaterials::DetectorMaterials(){
  
  // default values here
  fDetectorMedium = &mGasCF4;     //        default is gas
  fmediumDensity = DEFAULT_VALUE; // g/cm3  flagged as not set
  fmediumTemp = DEFAULT_VALUE;    // kelvin      medium temperature  
  fmediumPressure = DEFAULT_VALUE;// bar

  fmediumLY = DEFAULT_VALUE;  
  fmedium_VisFraction = DEFAULT_VALUE;   // flagged as not set
  fmediumLY_UV = DEFAULT_VALUE;          // default is even sharing
  fmediumLY_Vis = DEFAULT_VALUE;         
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
  } else if (med == "vacuum"){
    fDetectorMedium = &mVacuum;
  }
}

void DetectorMaterials::BuildMaterials(){
  int verb;
  bool flipWL;
  G4double scaleX, scaleY;
  G4double hc = CLHEP::h_Planck * CLHEP::c_light;
  
  G4cout << "\n-------- Build Materials() -----------\n" << G4endl;

  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();
  // -------------------- Chamber Materials --------------------//
  // -- Vacuum -- //
  std::vector<G4double> photonE_world = {0.1*eV, 12.4*eV}; // set n=1 everywhere
  std::vector<G4double> nWorld = {1.00, 1.00};   
  G4MaterialPropertiesTable *mptWorld = new G4MaterialPropertiesTable();
  mptWorld->AddProperty("RINDEX", photonE_world, nWorld); //2 is for length of arrays
  
  mVacuum = nist->FindOrBuildMaterial("G4_Galactic");
  mVacuum->SetMaterialPropertiesTable(mptWorld);
  
  // ----Stainless Steel material ---- //
  // steel for the walls (rindex = 0)
  G4Element* C  = new G4Element( "Carbon", "C",   6. , 12.011*g/mole);
  G4Element* Co = new G4Element( "Cobalt", "Co", 27. , 58.9332*g/mole);
  G4Element* Fe = new G4Element("Iron", "Fe", 26., 55.85*g/mole);
  mSteel_walls = new G4Material("ssteel_walls", 7.7*g/cm3, 3);
  mSteel_walls->AddElement(C, 0.04);
  mSteel_walls->AddElement(Fe, 0.88);
  mSteel_walls->AddElement(Co, 0.08);

  // steel for rest of the detector. Rindex not explicitly needed because surface should be used
  mSteel = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
  // https://web.mit.edu/8.13/8.13c/references-fall/aip/aip-handbook-section6g.pdf
  // https://refractiveindex.info/?shelf=3d&book=metals&page=iron
  // ^ these are for iron, not stainless. use LZ ref below which is for stainless apparently
  
  //addd ssteel optical property, by Ryan. Data are copied from LZ steelRefl
  std::vector<G4double> ssteel_PP = {0.60*eV, 11.27*eV};
  std::vector<G4double> ssteel_RIND = {1.07,1.07} ;
  std::vector<G4double> ssteel_ABSL =  {272.98*nm, 14.59*nm};
  G4MaterialPropertiesTable * mtp_ssteel =  new G4MaterialPropertiesTable();
  mtp_ssteel->AddProperty("RINDEX",ssteel_PP, ssteel_RIND);
  mtp_ssteel->AddProperty("ABSLENGTH",ssteel_PP, ssteel_ABSL);
  mSteel->SetMaterialPropertiesTable(mtp_ssteel);


  // -- stainless steel surface -- //
  // rough CF4 to stainless steel (side wall), alpha source body, rods
  opSteel_rodSurface = new G4OpticalSurface("opSteel_rodSurface");
  opSteel_rodSurface->SetModel(unified);
  opSteel_rodSurface->SetType(dielectric_metal); // only reflection or absorption
  opSteel_rodSurface->SetFinish(ground);
  // opSteel_rodSurface->SetPolish(1.0); // only for glisur model (not unified)
  opSteel_rodSurface->SetSigmaAlpha(90./4.*degree); // xxArb // this relates to the specular lobe variable. intuitively chosen

  // https://www.researchgate.net/figure/Total-diffuse-and-specular-reflectivity-of-a-a-stainless-steel-and-b-a-molybdenum_fig4_37459596
  // DOI: 10.1063/1.2202915, reflectivity of stainless steel is ~0.6-0.8 in visible spectrum, less at UV

  // https://www.thorlabs.com/newgrouppage9.cfm?objectgroup_id=13871
  // 40% reflectance down at 250nm, agrees with paper above

  G4MaterialPropertiesTable* mpt_ssteelSurface = new G4MaterialPropertiesTable();
  std::vector<G4double> ssteel_surface_PP   = {hc/(nm*1229.84), hc/(nm*995.97), hc/(770.16*nm), hc/(475.81*nm), hc/(403.23*nm), hc/(326.61*nm), hc/(254.*nm)};
  std::vector<G4double> ssteel_surface_reflectivity = {0.69, 0.66, 0.65, 0.60, 0.58, 0.51, 0.39}; // ratio of reflected, reflectivity = 1-absorbed-transmittance, transmittance=0 by default
  // std::vector<G4double> ssteel_surface_efficiency = {}; // ratio of absorbed // don't need to absorb (detect) in metal with efficiency. they just die
  // for (auto ref : ssteel_surface_reflectivity){ ssteel_surface_efficiency.push_back(1 - ref); }

  G4double evenSharing = 0.95/3; 
  // 0.95 for specular spike (below), zero everywhere else would give paper results (specular, otherwise lambertian)
  // change specularlobe to 0.10 for no real reason, and then set roughness with alpha above
  G4double facetLambertian = 0.1; // this makes extra effective lambertian reflection, because we aren't polished like the paper
  std::vector<G4double> ssteel_surface_SPECULARSPIKE =  {0.95-facetLambertian, 0.95-facetLambertian}; // reflection about average surface  
  std::vector<G4double> ssteel_surface_SPECULARLOBE =   {facetLambertian, facetLambertian};    // reflection about facet
  std::vector<G4double> ssteel_surface_BACKSCATTER =    {0.0, 0.0};     // reflection about average surface  
  // let these three be equal because who knows
  // from the reference above, diffuse reflection is always about 5% of the the total reflection (take the ratio of the images in the plot)
  // so we want all these constants to add up to 0.95, because DIFFUSELOBE = 1 - these three

  // Also SPECULARLOBECONSTANT, SPECULARSPIKECONSTANT, BACKSCATTERCONSTANT
  mpt_ssteelSurface->AddProperty("SPECULARSPIKECONSTANT", photonE_world, ssteel_surface_SPECULARSPIKE);
  mpt_ssteelSurface->AddProperty("SPECULARLOBECONSTANT", photonE_world, ssteel_surface_SPECULARLOBE);
  mpt_ssteelSurface->AddProperty("BACKSCATTERCONSTANT", photonE_world, ssteel_surface_BACKSCATTER);

  // mpt_ssteelSurface->AddProperty("EFFICIENCY", ssteel_surface_PP, ssteel_surface_efficiency);
  mpt_ssteelSurface->AddProperty("TRANSMITTANCE", photonE_world, {0, 0}); // no transmittance just to be safe (default is 0 anyways)
  mpt_ssteelSurface->AddProperty("REFLECTIVITY", ssteel_surface_PP, ssteel_surface_reflectivity);
  // mpt_ssteelSurface->AddProperty("REFLECTIVITY", photonE_world, {0, 0});
  opSteel_rodSurface->SetMaterialPropertiesTable(mpt_ssteelSurface);

  // -------- Aluminum -------- //
  G4String name, symbol; 
  G4double a, z, density, fractionmass, temperature, pressure;           
  G4int ncomponents, natoms;
  mAluminum = new G4Material("Aluminum", 13., 26.98*g/mole, 2.700*g/cm3);
  G4Element* Al = new G4Element(name="Aluminium"  ,symbol="Al" , z= 13., a=26.98*g/mole);
  mAluminum = new G4Material(name="MetalAluminium", density=2.700*g/cm3, ncomponents=1);
  mAluminum->AddElement(Al, 1);
  // mAluminum = nist->FindOrBuildMaterial("G4_ALUMINUM"); for whatever reason you can't customize this things MTP

  // atten length
  G4MaterialPropertiesTable *mpt_Aluminum = new G4MaterialPropertiesTable();
  std::string file_aluminumNindex = "dataOptics/aluminum_n_short.txt";
  std::vector<G4double> al_npp, al_n;
  OH::ReadAndFillArray(file_aluminumNindex, al_npp, al_n, verb = MG::GetVerbosity(), scaleX = nm, 1., flipWL = true);
  
  mpt_Aluminum->AddProperty("RINDEX", al_npp, al_n);
  mpt_Aluminum->AddProperty("GROUPVEL", photonE_world, {CLHEP::c_light*0.9, CLHEP::c_light*0.9});
  mAluminum->SetMaterialPropertiesTable(mpt_Aluminum);

  // -- aluminum surface -- 
  opAluminumSurface = new G4OpticalSurface("opSurf_aluminum", unified, ground, dielectric_metal);
  opAluminumSurface->SetSigmaAlpha(90./4.*degree); // randomly chosen
  G4MaterialPropertiesTable * mptAl_surface = new G4MaterialPropertiesTable();

  G4bool aluminum_PureFresnel = false;
  // if you want pure fresnel for shinny aluminum do below. you can change from ground to polished as well
  // otherwise use the 'non fresnel' reflectivity in vacuum that is scaled for being 'dirty'
  // aluminum probably too shinny but oh well. this gives pure fresnel
  if(aluminum_PureFresnel){
    std::string file_aluminumKindex = "dataOptics/aluminum_k_short.txt";
    std::vector<G4double> al_kpp, al_k;
    OH::ReadAndFillArray(file_aluminumKindex, al_kpp, al_k, verb = MG::GetVerbosity(), scaleX = nm, scaleY = 1., flipWL = true);
    mptAl_surface->AddProperty("REALRINDEX", al_npp, al_n);
    mptAl_surface->AddProperty("IMAGINARYRINDEX", al_kpp, al_k);
  } else {
    // if you want to scale the reflectivity for dirty aluminum, use below
    std::vector<G4double> alsurf_ref_x, alsurf_ref;
    OH::ReadAndFillArray("dataOptics/aluminum_calculatedRef.txt", alsurf_ref_x, alsurf_ref, verb = MG::GetVerbosity(), scaleX = nm, 1., flipWL = true);
    G4double scaleFresnelPerfect = 0.8;
    for (auto& ref : alsurf_ref) { ref *= scaleFresnelPerfect; }
    mptAl_surface->AddProperty("REFLECTIVITY", alsurf_ref_x, alsurf_ref);
  }

  // these are also picked somewhat randomly
  mpt_ssteelSurface->AddProperty("SPECULARSPIKECONSTANT", photonE_world, {0.7, 0.7}); // 70% of reflection is normal
  mpt_ssteelSurface->AddProperty("SPECULARLOBECONSTANT", photonE_world, {0.2, 0.2}); // 20% of reflection from the microfacets
  mpt_ssteelSurface->AddProperty("BACKSCATTERCONSTANT", photonE_world, {0, 0});   // so specular lobe (lambertian) is 10%
  opAluminumSurface->SetMaterialPropertiesTable(mptAl_surface);

  // no aluminum optical surface defined
  
  // -- black paper -- //
  // just using different steel with zero RINDEX so all absorbed
  // G4MaterialPropertiesTable * mptPaper = new G4MaterialPropertiesTable();
  // mBlackPaper = nist->FindOrBuildMaterial("G4_POLYSTYRENE");
  
  
  // -------------------- Misc Materials --------------------//
  // -- air -- //
  mAir = nist->FindOrBuildMaterial("G4_AIR");
  G4MaterialPropertiesTable *mptAir = new G4MaterialPropertiesTable();
  mptAir->AddProperty("RINDEX", "Air");

  // -- generic plastic -- //
  mGenericPlastic = nist->FindOrBuildMaterial("G4_POLYSTYRENE");
  G4MaterialPropertiesTable *mptPlastic = new G4MaterialPropertiesTable();
  mptPlastic->AddProperty("RINDEX", photonE_world, {1.6, 1.6}); // typical polystyrene
  mptPlastic->AddProperty("ABSLENGTH", photonE_world, {1.0*nm, 1.0*nm}); // absorb quickly
  mGenericPlastic->SetMaterialPropertiesTable(mptPlastic);
  
  // -- Lead -- //
  mLead = nist->FindOrBuildMaterial("G4_Pb");
  G4MaterialPropertiesTable *mptLead = new G4MaterialPropertiesTable();
  mptLead->AddProperty("RINDEX", photonE_world, {3.5, 3.5}); // high index metal
  mptLead->AddProperty("ABSLENGTH", photonE_world, {1.0*nm, 1.0*nm}); // absorb immediately
  mLead->SetMaterialPropertiesTable(mptLead);

  // -------------------- Smoke Detector --------------------//
  mSilver = nist->FindOrBuildMaterial("G4_Ag");
  G4MaterialPropertiesTable *mptSilver = new G4MaterialPropertiesTable();
  mptSilver->AddProperty("RINDEX", photonE_world, {3.0, 3.0}); // metal index
  mptSilver->AddProperty("ABSLENGTH", photonE_world, {1.0*nm, 1.0*nm}); // absorb immediately
  mSilver->SetMaterialPropertiesTable(mptSilver);
  
  mPalladium = nist->FindOrBuildMaterial("G4_Pd");
  G4MaterialPropertiesTable *mptPalladium = new G4MaterialPropertiesTable();
  mptPalladium->AddProperty("RINDEX", photonE_world, {3.0, 3.0}); // metal index
  mptPalladium->AddProperty("ABSLENGTH", photonE_world, {1.0*nm, 1.0*nm}); // absorb immediately
  mPalladium->SetMaterialPropertiesTable(mptPalladium);
  
  mGold = nist->FindOrBuildMaterial("G4_Au");
  G4MaterialPropertiesTable *mptGold = new G4MaterialPropertiesTable();
  mptGold->AddProperty("RINDEX", photonE_world, {3.0, 3.0}); // metal index
  mptGold->AddProperty("ABSLENGTH", photonE_world, {1.0*nm, 1.0*nm}); // absorb immediately
  mGold->SetMaterialPropertiesTable(mptGold);

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
  
  // Add optical properties to Am active matrix
  G4MaterialPropertiesTable *mptAmMatrix = new G4MaterialPropertiesTable();
  mptAmMatrix->AddProperty("RINDEX", photonE_world, {3.0, 3.0}); // metal-like index
  mptAmMatrix->AddProperty("ABSLENGTH", photonE_world, {1.0*nm, 1.0*nm}); // absorb immediately
  mAmActiveMatrix->SetMaterialPropertiesTable(mptAmMatrix);

  // -------------------- Generic CF4 stuff --------------------//
  // -- Elements in CF4 -- //
  G4Element * elCarbon = new G4Element("Carbon", "C", 6, 12.01 * g/mole);
  G4Element * elFluorine = new G4Element("Fluorine", "F", 9,  19. * g/mole);

  // could do density globally (not separate for gas, liquid) but won't for now...
  
  // -------------------- Gas CF4 --------------------//
  // default parameters baked in
  G4double cf4Gas_ly = 2500 /MeV;         // rough, taken from python work and slides
  G4double density_cf4Gas = 0.0022*g/cm3; // default value? need to make P and T function to get density
  G4double cf4Gas_temp = 160*kelvin;
  G4double cf4Gas_P = 0.5*bar;
  
  if(fDetectorMedium == &mGasCF4){

    // if using gas CF4, set values to those of medium (set via messenger)
    if(fmediumLY != DEFAULT_VALUE) cf4Gas_ly = fmediumLY;
    if(fmediumPressure != DEFAULT_VALUE) cf4Gas_P = fmediumPressure;
    if(fmediumTemp != DEFAULT_VALUE) cf4Gas_temp = fmediumTemp;

    // always set temperature/ pressure if available
    if(fmediumTemp != DEFAULT_VALUE) cf4Gas_temp = fmediumTemp;
    if(fmediumPressure != DEFAULT_VALUE) cf4Gas_P = fmediumPressure;

    // setting density logic
    if(fmediumDensity != DEFAULT_VALUE) density_cf4Gas = fmediumDensity; //  set density immediately!
    else if (fmediumTemp != DEFAULT_VALUE && fmediumPressure == DEFAULT_VALUE){
      // Set density from temperature only! Along saturation curve i think
      fmediumDensity = PH::GetDensity_data(fmediumTemp, "data/density_gasCF4.dat");
      density_cf4Gas = fmediumDensity;
      G4ExceptionDescription msg; msg << "Setting Gas density using some built in pressure, T= " << fmediumTemp/kelvin << " K, \n Rho = " << density_cf4Gas/(g/cm3) << " g/cm3\n"; 
      G4Exception("DetectorMaterials::BuildMaterials() - Gas CF4 option", "Austin's error message", JustWarning, msg);
    }

    else if(fmediumTemp != DEFAULT_VALUE && fmediumPressure != DEFAULT_VALUE && fmediumDensity == DEFAULT_VALUE){
      // USING REFPROP - R14 is CF4. this is good stuff!
      // arguments already converted to pure doubles with proper units (kelvin, kPa)
      fmediumDensity = PH::GetREFPROP_density("R14", fmediumTemp/kelvin, 100.*fmediumPressure/bar);
      density_cf4Gas = fmediumDensity;

      if(MG::GetVerbosity() > 0){
        G4ExceptionDescription msg; msg << "Setting Rho with REFPROP";
        msg << " T= " << fmediumTemp/kelvin << " K, P = " << fmediumPressure/bar << " bar\n";
        msg << "rho = " << density_cf4Gas/(g/cm3) << " g/cm3";
        G4Exception("DetectorMaterials::BuildMaterials() - Gas CF4 option", "Austin's error message", JustWarning, msg);
      }
    }

    // include elements
    mGasCF4 = new G4Material("cf4Gas", density_cf4Gas, 2, kStateGas, cf4Gas_temp, cf4Gas_P);
    mGasCF4->AddElement(elCarbon, 1);
    mGasCF4->AddElement(elFluorine, 4);

    // all physics happens in optics files
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
  
  if(fDetectorMedium == &mLiquidCF4){

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

    // setting density logic (same code as for gas!)
    if(fmediumDensity != DEFAULT_VALUE) density_cf4Liq = fmediumDensity; //  set density immediately!
    else if (fmediumTemp != DEFAULT_VALUE && fmediumPressure == DEFAULT_VALUE){
      // Set density from temperature only! Along saturation curve i think
      fmediumDensity = PH::GetDensity_data(fmediumTemp, "data/density_liquidCF4.dat");
      density_cf4Liq = fmediumDensity;
      G4ExceptionDescription msg; msg << "Setting Liquid density using some built in pressure, T= " << fmediumTemp/kelvin << " K, \n Rho = " << density_cf4Liq/(g/cm3) << " g/cm3\n"; 
      G4Exception("DetectorMaterials::BuildMaterials() - Liquid CF4 option", "Austin's error message", JustWarning, msg);
    }
    else if(fmediumTemp != DEFAULT_VALUE && fmediumPressure != DEFAULT_VALUE && fmediumDensity == DEFAULT_VALUE){
      // USING REFPROP - R14 is CF4. this is good stuff!
      // arguments already converted to pure doubles with proper units (kelvin, kPa)
      fmediumDensity = PH::GetREFPROP_density("R14", fmediumTemp/kelvin, 100.*fmediumPressure/bar);
      density_cf4Liq = fmediumDensity;

      if(MG::GetVerbosity() > 0){
        G4ExceptionDescription msg; msg << "Setting Rho with REFPROP";
        msg << " T= " << fmediumTemp/kelvin << " K, P = " << fmediumPressure/bar << " bar\n";
        msg << "P = " << density_cf4Liq/(g/cm3) << " g/cm3";
        G4Exception("DetectorMaterials::BuildMaterials() - Liquid CF4 option", "Austin's error message", JustWarning, msg);
      }
    }

    mLiquidCF4 = new G4Material("cf4Liq", density_cf4Liq, 2, kStateLiquid, cf4Liq_temp, cf4Liq_P);
    mLiquidCF4->AddElement(elCarbon, 1);
    mLiquidCF4->AddElement(elFluorine, 4);
    #include "Optics/LiquidCF4.icc" // all the physics is in here!
    // #include "Optics/oldLiquidCF4.icc"
  }
  
  
  // -------------------- GAr --------------------//
  //


  // -------------------- Liquid Xenon --------------------//
  mLXe = new G4Material("LXe", 54., 131.29 * g / mole, 3.020 * g / cm3);
  G4MaterialPropertiesTable * mptLXe = new G4MaterialPropertiesTable();
  std::vector<G4double> photonE_lxe, n_lxe;
  OH::ReadAndFillArray("dataOptics/lxe_n_merged_wl.txt", photonE_lxe, n_lxe, verb = MG::GetVerbosity(), scaleX = nm, 1., flipWL = true); // scale um to nm, flip wavelengths to eV
  mptLXe->AddProperty("RINDEX", photonE_lxe, n_lxe);
  mLXe->SetMaterialPropertiesTable(mptLXe);


  // -------------------- SiPM Stuff --------------------//
  // -- Ceramic --//
  //ceramic(Medical Physics, May 2000), from old BrachyMaterial class
  mCeramic = new G4Material("Ceramic", 2.88*g/cm3, 2, kStateSolid);
  mCeramic->AddElement(nist->FindOrBuildElement("Al"),2);
  mCeramic->AddElement(nist->FindOrBuildElement("O"),3);
  
  // Add optical properties to ceramic
  G4MaterialPropertiesTable *mptCeramic = new G4MaterialPropertiesTable();
  mptCeramic->AddProperty("RINDEX", photonE_world, {1.8, 1.8}); // ceramic index
  mCeramic->SetMaterialPropertiesTable(mptCeramic);
  
  // add diffuse reflection here
  G4OpticalSurface * opSurf_ceramic = new G4OpticalSurface("opSurf_ceramicDiffuser", unified, groundfrontpainted, dielectric_dielectric);
  // from a quick google search, it looks like *most* ceramics go from 20% ish below 400ish nm
  // quickly up to around 90% reflectivity at 500nm
  
  G4MaterialPropertiesTable *mpt_Ceramic_optical = new G4MaterialPropertiesTable();
  std::vector<G4double> ceramicDiffusion = {0.9, 0.8, 0.15};
  if(DETECTORSETUP::GetDiffusionStatus() == false) ceramicDiffusion = {0, 0, 0};
  mpt_Ceramic_optical->AddProperty("REFLECTIVITY", {hc/(1100*nm), hc/(500*nm), hc/(400*nm)}, ceramicDiffusion);
  mpt_Ceramic_optical->AddProperty("TRANSMITTANCE", photonE_world, {0., 0}); // make sure they stop (0 is also default behaviour)
  opSurf_ceramic->SetMaterialPropertiesTable(mpt_Ceramic_optical);

  // -------------------- Quartz/SiO2 --------------------//
  // -- Quartz/SiO2 Bulk -- //
  mQuartz = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE");
  G4MaterialPropertiesTable *mptQuartz = new G4MaterialPropertiesTable();

  std::vector<G4double> photonE_nQuartz, nQuartz, photonE_kQuartz, kQuartz;
  OH::ReadAndFillArray("dataOptics/sio2_n_wl.txt", photonE_nQuartz, nQuartz, verb = MG::GetVerbosity(), scaleX = nm, 1., flipWL = true); // no scaling, flip wavelengths to eV
  // OH::ReadAndFillArray("dataOptics/sio2_k_wl.txt", photonE_kQuartz, kQuartz, verb = MG::GetVerbosity(), scaleX = nm, 1., flipWL = true); // no scaling, flip wavelengths to eV
  mptQuartz->AddProperty("RINDEX", photonE_nQuartz, nQuartz);

  std::vector<G4double> photonE_absLQuartz, absLQuartz;
  OH::ReadAndFillArray("dataOptics/quartz_absLength.txt", photonE_absLQuartz, absLQuartz, verb = MG::GetVerbosity(), scaleX = nm, scaleY = um, flipWL = true); // no scaling, flip wavelengths to eV
  if(DETECTORSETUP::GetQuartzABSstatus() && !DETECTORSETUP::GetVisibleStatus()){ mptQuartz->AddProperty("ABSLENGTH", photonE_absLQuartz, absLQuartz);}
  
  mQuartz->SetMaterialPropertiesTable(mptQuartz);

  // -------------------- General Surfaces --------------------//
  //-- perfect transmission --//
  opSurf_perfectTrans = new G4OpticalSurface("perfectTrans_opSurf");
  opSurf_perfectTrans->SetModel(unified);
  opSurf_perfectTrans->SetType(dielectric_metal);
  opSurf_perfectTrans->SetFinish(polished); //could also use dielectric_dielectric -> polished front painted
  opSurf_perfectTrans->SetSigmaAlpha(0.); // perfectly smooth
  
  G4double t = 1.0;
  // std::vector<G4double> photonE_world = {1.0*eV, 12.4*eV};
  std::vector<G4double> transPerfect = {t, t};
  std::vector<G4double> noRef = {1.0-t, 1.0-t};

  G4MaterialPropertiesTable* mpt_perfectTrans = new G4MaterialPropertiesTable();
  mpt_perfectTrans->AddProperty("TRANSMITTANCE", photonE_world, transPerfect);
  mpt_perfectTrans->AddProperty("REFLECTIVITY", photonE_world, noRef);
  opSurf_perfectTrans->SetMaterialPropertiesTable(mpt_perfectTrans);

  //-- perfect reflection --//
  opSurf_perfectRef = new G4OpticalSurface("perfectTrans_opSurf", unified, polished, dielectric_metal);
  opSurf_perfectRef->SetSigmaAlpha(0.); // perfectly smooth.  //could also use dielectric_dielectric -> polishedfrontpainted
  G4MaterialPropertiesTable* mpt_perfectRef = new G4MaterialPropertiesTable();
  mpt_perfectRef->AddProperty("TRANSMITTANCE", photonE_world, noRef);
  mpt_perfectRef->AddProperty("REFLECTIVITY", photonE_world, transPerfect);
  opSurf_perfectRef->SetMaterialPropertiesTable(mpt_perfectRef);

  //-- perfect absorber --//
  G4double paperRef = 0.05; // 5% reflection, 95% absorption
  opSurf_perfectAbsorber = new G4OpticalSurface("absorber_opSurf", unified, polished, dielectric_metal);
  opSurf_perfectAbsorber->SetSigmaAlpha(0.); // perfectly smooth.  //could also use dielectric_dielectric -> polishedfrontpainted
  
  G4MaterialPropertiesTable* mpt_perfectAbsorber = new G4MaterialPropertiesTable();
  mpt_perfectAbsorber->AddProperty("TRANSMITTANCE", photonE_world, noRef);
  mpt_perfectAbsorber->AddProperty("REFLECTIVITY", photonE_world, {paperRef, paperRef});
  mpt_perfectAbsorber->AddProperty("ABSLENGTH", photonE_world, {1.0*nm, 1.0*nm});
  opSurf_perfectAbsorber->SetMaterialPropertiesTable(mpt_perfectAbsorber);

  // -------------------- Silicon -------------------- //
  // -- Silicon Bulk -- //
  mSilicon = nist->FindOrBuildMaterial("G4_Si");
  std::vector<G4double> photonE_nSilicon, nSi, photonE_kSilicon, kSilicon;

  std::string silicon_nFile = "dataOptics/silicon_merged_n_wl.txt";
  std::string silicon_kFile = "dataOptics/silicon_merged_k_wl.txt";
  OH::ReadAndFillArray(silicon_nFile, photonE_nSilicon, nSi, verb = MG::GetVerbosity(), scaleX = nm, 1., flipWL = true); // scale um to nm, flip wavelengths to eV
  OH::ReadAndFillArray(silicon_kFile, photonE_kSilicon, kSilicon, verb = MG::GetVerbosity() , scaleX = nm, 1., flipWL = true); // scale um to nm, flip wavelengths to eV

  G4MaterialPropertiesTable *mptSi = new G4MaterialPropertiesTable();
//   mptSi->AddProperty("RINDEX", photonE_nSilicon, nSi);

  // cheating to fix group velocities, because n<1 is not allowed by G4
  std::vector<G4double> photonE_gv = {1.0*eV, 12*eV};
  std::vector<G4double> gv = {CLHEP::c_light*0.9, CLHEP::c_light*0.9};
  mptSi->AddProperty("GROUPVEL", photonE_gv, gv);

  // do this so photons dont step to edge of silicon. shouldnt matter because of
  // processHits killing the track, but good to do anyways
  mptSi->AddProperty("ABSLENGTH", photonE_world, {1.0*nm, 1.0*nm});
  mSilicon->SetMaterialPropertiesTable(mptSi);

  // -- silicon optical surface --//
  opSurf_silicon = new G4OpticalSurface("silicon_opSurf", unified, polished, dielectric_metal);
  opSurf_silicon->SetSigmaAlpha(0.);

  G4MaterialPropertiesTable *mpt_siliconSurf = new G4MaterialPropertiesTable();
//   mpt_siliconSurf->AddProperty("REALRINDEX", photonE_nSilicon, nSi);
  // mpt_siliconSurf->AddProperty("REALRINDEX", photonE_world, {20, 20}); // xx
//   mpt_siliconSurf->AddProperty("IMAGINARYRINDEX", photonE_kSilicon, kSilicon);
  // mpt_siliconSurf->AddProperty("IMAGINARYRINDEX", photonE_world, {20, 20}); // xx
  // mpt_siliconSurf->AddProperty("REFLECTIVITY", photonE_world, {0, 0}); // you can't do the absorptive scaling here. it won't calculate with index values
    mpt_siliconSurf->AddProperty("TRANSMITTANCE", photonE_world, transPerfect); // needed to get the nonR photons inside the bulk
  opSurf_silicon->SetMaterialPropertiesTable(mpt_siliconSurf);

  
  // -------------------- Longpass Filter --------------------//
  std::vector<G4double> rindexFilterTop = {1.74, 1.74}; // effective index for surface but not for glass body (which is quartz)
  // std::vector<G4double> rindexBody = {1.36, 1.36}; // effective index for body but probably not for surface. 
  // this is just quartz above

  // heavy quartz for LP filter
  mBoroSilicate = nist->FindOrBuildMaterial("G4_BORON_OXIDE");
  G4MaterialPropertiesTable* mptFilterTop = new G4MaterialPropertiesTable();
  std::vector<G4double> rindexTop = {1.74, 1.74}; // effective index for surface but probably not for glass body
  mptFilterTop->AddProperty("RINDEX", photonE_world, rindexTop);
  mBoroSilicate->SetMaterialPropertiesTable(mptFilterTop);

  //-- optical surfaces for filter --//
  std::vector<G4double> wlsE_filter, transmission_filter, reflectivity_filter; // get out photon energy and transmission
  // G4bool flipWL; int verb; G4double scaleX, scaleY;
  OH::ReadAndFillArray("dataOptics/lpfilter_500nm.txt", wlsE_filter, transmission_filter, verb = MG::GetVerbosity(), scaleX = 1.*nm, scaleY = 1./100., flipWL = true); // scale transmission values by 100, flip wavelengths to eV
  for (std::vector<G4double>::iterator it = transmission_filter.begin(); it != transmission_filter.end(); it++){
    reflectivity_filter.push_back(1.0 - *it);
  }
  
  // medium <--> surface
  opSurf_lpFilter = new G4OpticalSurface("opSurf_lpFilter");
  opSurf_lpFilter->SetModel(unified);
  // opSurf_lpFilter->SetType(dielectric_dielectric);
  // opSurf_lpFilter->SetFinish(polishedfrontpainted);
  opSurf_lpFilter->SetType(dielectric_metal);
  opSurf_lpFilter->SetFinish(polished);
  opSurf_lpFilter->SetSigmaAlpha(0.);

  G4MaterialPropertiesTable* mptFilterSurfaceTop = new G4MaterialPropertiesTable();
  mptFilterSurfaceTop->AddProperty("REFLECTIVITY", wlsE_filter, reflectivity_filter);
  // mptFilterSurfaceTop->AddProperty("REFLECTIVITY", photonE_world, {0., 0.});
  // mptFilterSurfaceTop->AddProperty("TRANSMITTANCE", wlsE_filter, transmission_filter); // this is wrong. Reflectivity does the physics, Transmittance does the absorption
  mptFilterSurfaceTop->AddProperty("TRANSMITTANCE", photonE_world, {1., 1.});
  opSurf_lpFilter->SetMaterialPropertiesTable(mptFilterSurfaceTop);
  // mptFilterSurfaceTop->DumpTable();
  // exit(0);

  return;
}











// end