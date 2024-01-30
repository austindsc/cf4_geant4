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
  // rough CF4 to stainless steel (side wall)
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
  std::vector<G4double> ssteel_surface_efficiency = {}; // ratio of absorbed
  for (auto ref : ssteel_surface_reflectivity){ ssteel_surface_efficiency.push_back(1 - ref); }

  std::vector<G4double> ssteel_surface_simplePP = {1.*eV, 10.*eV};
  G4double evenSharing = 0.95/3; 
  // 0.95 for specular spike (below), zero everywhere else would give paper results (specular, otherwise lambertian)
  // change specularlobe to 0.10 for no real reason, and then set roughness with alpha above
  G4double facetLambertian = 0.1; //xxArb // this makes extra effective lambertian reflection, because we aren't polished like the paper
  std::vector<G4double> ssteel_surface_SPECULARSPIKE = {0.95-facetLambertian, 0.95-facetLambertian}; // reflection about average surface  
  std::vector<G4double> ssteel_surface_SPECULARLOBE = {facetLambertian, facetLambertian};    // reflection about facet
  std::vector<G4double> ssteel_surface_BACKSCATTER = {0.0, 0.0};     // reflection about average surface  
  // let these three be equal because who knows
  // from the reference above, diffuse reflection is always about 5% of the the total reflection (take the ratio of the images in the plot)
  // so we want all these constants to add up to 0.95, because DIFFUSELOBE = 1 - these three

  // Also SPECULARLOBECONSTANT, SPECULARSPIKECONSTANT, BACKSCATTERCONSTANT
  mpt_ssteelSurface->AddProperty("SPECULARSPIKECONSTANT", ssteel_surface_simplePP, ssteel_surface_SPECULARSPIKE);
  mpt_ssteelSurface->AddProperty("SPECULARLOBECONSTANT", ssteel_surface_simplePP, ssteel_surface_SPECULARLOBE);
  mpt_ssteelSurface->AddProperty("BACKSCATTERCONSTANT", ssteel_surface_simplePP, ssteel_surface_BACKSCATTER);

  mpt_ssteelSurface->AddProperty("EFFICIENCY", ssteel_surface_PP, ssteel_surface_efficiency);
  mpt_ssteelSurface->AddProperty("REFLECTIVITY", ssteel_surface_PP, ssteel_surface_reflectivity);
  opSteel_rodSurface->SetMaterialPropertiesTable(mpt_ssteelSurface);

  // -------- Aluminum -------- //

  // aluminium (below from Ryan and Zhiheng's code, Nov 14 2023)
  G4String name, symbol; 
  G4double a, z, density, fractionmass, temperature, pressure;           
  G4int ncomponents, natoms;
  // mAluminum = new G4Material("Aluminum", 13., 26.98*g/mole, 2.700*g/cm3);
  G4Element* Al = new G4Element(name="Aluminium"  ,symbol="Al" , z= 13., a=26.98*g/mole);
  mAluminum = new G4Material(name="MetalAluminium", density=2.700*g/cm3, ncomponents=1);
  mAluminum->AddElement(Al, 1);

  // mAluminum = nist->FindOrBuildMaterial("G4_ALUMINUM");

  //Aluminium optical properties added by Ryan ////////////////
  ///https://refractiveindex.info/?shelf=main&book=Si&page=Aspnes////
  // const G4int Al_NUMENTRIES = 200;
  std::vector<G4double> Al_PP = {0.005*eV,0.00521*eV,0.00543*eV,0.00567*eV,0.00591*eV,0.00616*eV,0.00642*eV,0.00669*eV,0.00698*eV,0.00728*eV,0.00759*eV,0.00791*eV,0.00824*eV,0.0086*eV,0.00896*eV,0.00934*eV,
    0.00974*eV,0.0102*eV,0.0106*eV,0.011*eV,0.0115*eV,0.012*eV,0.0125*eV,0.013*eV,0.0136*eV,0.0142*eV,0.0148*eV,0.0154*eV,0.0161*eV,0.0167*eV,0.0175*eV,0.0182*eV,0.019*eV,0.0198*eV,0.0206*eV,0.0215*eV,0.0224*eV,
    0.0234*eV,0.0244*eV,0.0254*eV,0.0265*eV,0.0276*eV,0.0288*eV,0.03*eV,0.0313*eV,0.0326*eV,0.034*eV,0.0355*eV,0.037*eV,0.0385*eV,0.0402*eV,0.0419*eV,0.0437*eV,0.0455*eV,0.0475*eV,0.0495*eV,0.0516*eV,0.0538*eV,
    0.0561*eV,0.0585*eV,0.061*eV,0.0635*eV,0.0663*eV,0.0691*eV,0.072*eV,0.0751*eV,0.0783*eV,0.0816*eV,0.0851*eV,0.0887*eV,0.0925*eV,0.0964*eV,0.101*eV,0.105*eV,0.109*eV,0.114*eV,0.119*eV,0.124*eV,0.129*eV,0.135*eV,
    0.14*eV,0.146*eV,0.152*eV,0.159*eV,0.166*eV,0.173*eV,0.18*eV,0.188*eV,0.196*eV,0.204*eV,0.213*eV,0.222*eV,0.231*eV,0.241*eV,0.251*eV,0.262*eV,0.273*eV,0.285*eV,0.297*eV,0.31*eV,0.323*eV,0.337*eV,0.351*eV,0.366*eV,
    0.381*eV,0.398*eV,0.415*eV,0.432*eV,0.451*eV,0.47*eV,0.49*eV,0.511*eV,0.532*eV,0.555*eV,0.579*eV,0.603*eV,0.629*eV,0.656*eV,0.684*eV,0.713*eV,0.743*eV,0.775*eV,0.808*eV,0.842*eV,0.878*eV,0.915*eV,0.954*eV,0.995*eV,
    1.04*eV,1.08*eV,1.13*eV,1.18*eV,1.23*eV,1.28*eV,1.33*eV,1.39*eV,1.45*eV,1.51*eV,1.57*eV,1.64*eV,1.71*eV,1.78*eV,1.86*eV,1.94*eV,2.02*eV,2.11*eV,2.2*eV,2.29*eV,2.39*eV,2.49*eV,2.59*eV,2.71*eV,2.82*eV,2.94*eV,3.07*eV,
    3.2*eV,3.33*eV,3.47*eV,3.62*eV,3.78*eV,3.94*eV,4.1*eV,4.28*eV,4.46*eV,4.65*eV,4.85*eV,5.05*eV,5.27*eV,5.49*eV,5.73*eV,5.97*eV,6.23*eV,6.49*eV,6.77*eV,7.06*eV,7.36*eV,7.67*eV,7.99*eV,8.34*eV,8.69*eV,9.06*eV,9.44*eV,
    9.85*eV,10.3*eV,10.7*eV,11.2*eV,11.6*eV,12.1*eV,12.6*eV,13.2*eV,13.7*eV,14.3*eV,14.9*eV,15.6*eV,16.2*eV,16.9*eV,17.6*eV,18.4*eV,19.2*eV,20*eV};
  
  std::vector<G4double> Al_RIND ={473,463,452,441,431,420,410,401,391,381,372,363,353,345,336,327,319,310,302,294,286,278,270,262,255,247,240,233,225,218,211,205,198,191,185,178,172,166,159,153,147,142,136,
    130,125,119,114,109,104,99.1,94.4,89.8,85.4,81.1,77,73.1,69.2,65.6,62.1,58.8,55.6,52.6,49.7,47,44.4,42,39.7,37.6,35.6,33.7,31.9,30.2,28.7,27.2,25.8,24.6,23.4,22.2,21.2,20.1,19.2,
    18.3,17.4,16.6,15.8,15,14.3,13.5,12.8,12.2,11.5,10.9,10.3,9.72,9.16,8.63,8.11,7.62,7.15,6.7,6.28,5.87,5.49,5.13,4.79,4.47,4.17,3.88,3.62,3.38,3.15,2.93,2.74,2.56,2.39,2.23,2.09,1.97,
    1.85,1.75,1.65,1.57,1.5,1.44,1.39,1.35,1.32,1.3,1.3,1.31,1.34,1.4,1.48,1.61,1.81,2.09,2.45,2.71,2.65,2.31,1.91,1.62,1.43,1.3,1.19,1.1,1.02,0.935,0.857,0.782,0.711,0.644,0.583,0.528,
    0.478,0.433,0.393,0.358,0.326,0.298,0.273,0.251,0.231,0.213,0.197,0.182,0.169,0.156,0.145,0.134,0.124,0.115,0.106,0.0976,0.0899,0.0827,0.0761,0.07,0.0646,0.0596,0.0553,0.0515,0.0482,
    0.0454,0.043,0.0412,0.0399,0.0391,0.0392,0.0406,0.0444,0.0553,0.116,0.282,0.39,0.469,0.531,0.583,0.626,0.664};//refractive index

  std::vector<G4double> Al_ABSL ={37.6*nm,36.8*nm,36*nm,35.2*nm,34.4*nm,33.6*nm,32.8*nm,32.1*nm,31.4*nm,30.7*nm,30*nm,29.3*nm,28.6*nm,28*nm,27.3*nm,26.7*nm,26.1*nm,25.5*nm,24.9*nm,24.3*nm,23.8*nm,23.2*nm,22.7*nm,22.2*nm,
    21.7*nm,21.2*nm,20.7*nm,20.2*nm,19.7*nm,19.3*nm,18.9*nm,18.4*nm,18*nm,17.6*nm,17.2*nm,16.8*nm,16.5*nm,16.1*nm,15.8*nm,15.5*nm,15.1*nm,14.8*nm,14.5*nm,14.2*nm,14*nm,13.7*nm,13.5*nm,13.2*nm,13*nm,12.8*nm,12.6*nm,12.4*nm,
    12.2*nm,12*nm,11.8*nm,11.7*nm,11.5*nm,11.4*nm,11.2*nm,11.1*nm,11*nm,10.9*nm,10.7*nm,10.6*nm,10.5*nm,10.5*nm,10.4*nm,10.3*nm,10.2*nm,10.1*nm,10.1*nm,10*nm,9.93*nm,9.86*nm,9.8*nm,9.73*nm,9.67*nm,9.61*nm,9.55*nm,9.48*nm,
    9.42*nm,9.36*nm,9.29*nm,9.23*nm,9.17*nm,9.1*nm,9.04*nm,8.97*nm,8.91*nm,8.85*nm,8.79*nm,8.73*nm,8.67*nm,8.62*nm,8.56*nm,8.51*nm,8.46*nm,8.42*nm,8.37*nm,8.33*nm,8.3*nm,8.26*nm,8.23*nm,8.2*nm,8.17*nm,8.14*nm,8.12*nm,8.1*nm,
    8.08*nm,8.07*nm,8.06*nm,8.05*nm,8.04*nm,8.03*nm,8.03*nm,8.03*nm,8.03*nm,8.04*nm,8.04*nm,8.05*nm,8.07*nm,8.08*nm,8.1*nm,8.12*nm,8.15*nm,8.18*nm,8.22*nm,8.26*nm,8.31*nm,8.36*nm,8.42*nm,8.49*nm,8.57*nm,8.65*nm,8.72*nm,8.71*nm,
    8.52*nm,8.06*nm,7.5*nm,7.11*nm,6.96*nm,6.92*nm,6.92*nm,6.92*nm,6.91*nm,6.88*nm,6.86*nm,6.83*nm,6.8*nm,6.78*nm,6.77*nm,6.76*nm,6.75*nm,6.75*nm,6.75*nm,6.76*nm,6.77*nm,6.79*nm,6.8*nm,6.82*nm,6.84*nm,6.86*nm,6.89*nm,6.91*nm,
    6.94*nm,6.97*nm,7.01*nm,7.04*nm,7.08*nm,7.13*nm,7.18*nm,7.24*nm,7.3*nm,7.37*nm,7.45*nm,7.54*nm,7.65*nm,7.77*nm,7.9*nm,8.06*nm,8.25*nm,8.47*nm,8.72*nm,9.03*nm,9.4*nm,9.86*nm,10.4*nm,11.2*nm,12.3*nm,13.9*nm,16.6*nm,22.5*nm,
    51.4*nm,136*nm,206*nm,269*nm,332*nm,397*nm,464*nm,536*nm};

  // atten length
  G4MaterialPropertiesTable *mpt_Aluminum = new G4MaterialPropertiesTable();
  mpt_Aluminum->AddProperty("RINDEX", Al_PP, Al_RIND);
  mpt_Aluminum->AddProperty("ABSLENGTH", Al_PP, Al_ABSL);
  mpt_Aluminum->AddProperty("GROUPVEL", photonE_world, {CLHEP::c_light*0.9, CLHEP::c_light*0.9});
  mAluminum->SetMaterialPropertiesTable(mpt_Aluminum);

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

  // -------------------- Silicon -------------------- //
  // -- Silicon Bulk -- //
  mSilicon = nist->FindOrBuildMaterial("G4_Si");
  std::vector<G4double> photonE_nSilicon, nSi, photonE_kSilicon, kSilicon;

  std::string silicon_nFile = "dataOptics/silicon_merged_n_wl.txt";
  std::string silicon_kFile = "dataOptics/silicon_merged_k_wl.txt";
  OH::ReadAndFillArray(silicon_nFile, photonE_nSilicon, nSi, verb = MG::GetVerbosity(), scaleX = nm, 1., flipWL = true); // scale um to nm, flip wavelengths to eV
  OH::ReadAndFillArray(silicon_kFile, photonE_kSilicon, kSilicon, verb = MG::GetVerbosity() , scaleX = nm, 1., flipWL = true); // scale um to nm, flip wavelengths to eV

  G4MaterialPropertiesTable *mptSi = new G4MaterialPropertiesTable();
  mptSi->AddProperty("RINDEX", photonE_nSilicon, nSi);

  // cheating to fix group velocities, because n<1 is not allowed by G4
  std::vector<G4double> photonE_gv = {1.0*eV, 12*eV};
  std::vector<G4double> gv = {CLHEP::c_light*0.9, CLHEP::c_light*0.9};
  mptSi->AddProperty("GROUPVEL", photonE_gv, gv);

  // cheating to get photons to stop in silicon so our ProcessHits() method works only when photon is IN silicon
  mptSi->AddProperty("ABSLENGTH", photonE_world, {1.0*nm, 1.0*nm});
  mSilicon->SetMaterialPropertiesTable(mptSi);

  // -- silicon optical surface --//
  opSurf_silicon = new G4OpticalSurface("silicon_opSurf", unified, polished, dielectric_metal);
  opSurf_silicon->SetSigmaAlpha(0.);

  G4MaterialPropertiesTable *mpt_siliconSurf = new G4MaterialPropertiesTable();
  mpt_siliconSurf->AddProperty("REALRINDEX", photonE_nSilicon, nSi);
  mpt_siliconSurf->AddProperty("IMAGINARYRINDEX", photonE_kSilicon, kSilicon);
  // mpt_siliconSurf->AddProperty("REFLECTIVITY", photonE_world, {0, 0}); // you can't do the absorptive scaling here. it won't calculate with index values
  mpt_siliconSurf->AddProperty("TRANSMITTANCE", photonE_world, transPerfect); // needed to get the nonR photons inside the bulk
  opSurf_silicon->SetMaterialPropertiesTable(mpt_siliconSurf);

  // ------ correction surfaces ------ //
  G4double Rdiffuse = 0.17; // should be 0.17, from nEXO vacuum paper  
  if(DETECTORSETUP::GetDiffusionStatus() == false){Rdiffuse = 0.0;}
  opSurf_myCorrector1_diffuserIn = new G4OpticalSurface("opSurf_myCorrector1_diffuserIn", unified, groundfrontpainted, dielectric_dielectric);
  // groundfrontpainted is only diffuse reflection. set by REFLECTIVITY
  // absorption then is 1 - TRANSMITTANCE, default is T = 0

  G4MaterialPropertiesTable *mpt_myCorrector1_diffuseIn = new G4MaterialPropertiesTable();  
  mpt_myCorrector1_diffuseIn->AddProperty("TRANSMITTANCE", photonE_world, transPerfect); // get everything in (after doing diffuse reflection), ABS = 1 - TRANSMITTANCE = 0
  mpt_myCorrector1_diffuseIn->AddProperty("REFLECTIVITY", photonE_world, {Rdiffuse, Rdiffuse}); // how much diffusion do we want
  opSurf_myCorrector1_diffuserIn->SetMaterialPropertiesTable(mpt_myCorrector1_diffuseIn);

  opSurf_myCorrector2_absorptionOut = new G4OpticalSurface("opSurf_myCorrector2_absorptionOut", unified, polishedfrontpainted, dielectric_dielectric);
  // std::vector<G4double> correctRwithAbs_transmission = {0., 0.}; // load in these values from my calculations
  std::vector<G4double> xphotonE_TransCorrection, y_TransCorrection;
  OH::ReadAndFillArray("dataOptics/effective_correction_transmission.txt", xphotonE_TransCorrection, y_TransCorrection, verb = MG::GetVerbosity(), scaleX = nm, scaleY = 1., flipWL = true); // scale um to nm, flip wavelengths to eV

  G4MaterialPropertiesTable *mpt_myCorrector2_absorptionOut = new G4MaterialPropertiesTable();
  mpt_myCorrector2_absorptionOut->AddProperty("TRANSMITTANCE", xphotonE_TransCorrection, y_TransCorrection); // absorb certain amount of SiPM reflections if T < 1
  mpt_myCorrector2_absorptionOut->AddProperty("REFLECTIVITY", photonE_world, noRef); // no reflections
  opSurf_myCorrector2_absorptionOut->SetMaterialPropertiesTable(mpt_myCorrector2_absorptionOut);
  
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