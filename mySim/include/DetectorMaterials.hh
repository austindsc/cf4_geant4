#ifndef DETECTORMATERIALS_H
#define DETECTORMATERIALS_H 1

#include "globals.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

#ifndef DEFAULT_VALUE
#define DEFAULT_VALUE -69
#endif

class G4Material;
class G4String;
class G4OpticalSurface;

class DetectorMaterials{
  friend class CF4DetectorConstruction;
  friend class MaterialsMessenger;
  friend class collimator;
  friend class LongPassFilter;
  friend class alphaSource1;
  friend class VUV4;
  
public:
  void BuildMaterials(); //call this after setting variables with messenger

  void SetDetectionMedium(G4String med);
  G4String GetDetectorMediumString(){return fmedString; }
  G4Material * GetDetectorMedium(){return *fDetectorMedium; }
  
  void SetMediumDensity(G4double r){ fmediumDensity = r; }
  G4double GetMediumDensity(){ return fmediumDensity; }
  
  void SetMediumLY(G4double ly){ if(ly == 0/MeV) ly = 0.001/MeV; fmediumLY = ly; } //must be non zero. Just make small
  G4double GetMediumLY(){ return fmediumLY; }

  void SetMediumLY_VisFraction(G4double fraction){ fmedium_VisFraction = fraction; }
  G4double GetMedium_VisFraction(){ return fmedium_VisFraction; }

  void SetMediumLY_UV(G4double ly){ fmediumLY_UV = ly;}
  G4double GetMediumLY_UV(){ return fmediumLY_UV; }

  void SetMediumLY_Vis(G4double ly){ fmediumLY_Vis = ly;}
  G4double GetMediumLY_Vis(){ return fmediumLY_Vis; }
  
  void SetMediumTemp(G4double temp){ fmediumTemp = temp; }
  G4double GetMediumTemp(){ return fmediumTemp; }
    
  // singleton Stuff
  DetectorMaterials(DetectorMaterials &other) = delete; // not cloneable
  void operator=(const DetectorMaterials &) = delete;   // not assignable
  
  static DetectorMaterials *GetInstance();
  
protected:
  DetectorMaterials();
  static DetectorMaterials* singleton;
  virtual ~DetectorMaterials();
  
  // chamber materials
  G4Material * mVacuum; // vacuum
  G4Material * mAir;
  G4Material * mWalls; // black absorptive paper stuff  
  G4Material * mSteel;  // stainless
  G4Material * mSteel_walls;  // stainless
  G4Material * mLead;
  G4Material * mGenericPlastic;
  G4Material * mAluminum;
  // G4Material * mBlackPaper;
  
  // smoke detector (alpha source)
  G4Material * mAmActiveMatrix; // gold mixed with AmO2
  G4Material * mSilver;
  G4Material * mGold;
  G4Material * mPalladium;
  
  // for materials messenger
  G4String fmedString;
  G4Material ** fDetectorMedium;// what is currently in active volume
  
  //--- different detection mediums
  G4Material * mGasCF4;
  G4Material * mLiquidCF4;
  G4Material * mGasAr;
  G4Material * mLXe;
  
  // and their properties
  G4double fmediumLY;
  G4double fmediumLY_UV;
  G4double fmediumLY_Vis;
  G4double fmedium_VisFraction;

  G4double fmediumDensity;
  G4double fmediumTemp;
  G4double fmediumPressure;

  // SiPM stuff
  G4Material * mSilicon;
  G4Material * mQuartz; // this is SiO2
  // G4Material * mHeavyQuartz; // top of optical filter
  G4Material * mBoroSilicate; // for filter
  G4Material * mCeramic;

  ////// --- Optical Surfaces ---//////
  // optical property tables (for surfaces and skins, excluding for the LP filter)
  G4OpticalSurface * opSteel_rodSurface;
  G4OpticalSurface * opAl_surface;

  // -- sipm optical surfaces -- //
  G4OpticalSurface * opSurf_Air;
  G4OpticalSurface * opSurf_Quartz;       // normal quartz (using n only)
  // G4OpticalSurface * opSurf_surfaceQuartz;// if surface of sipm quartz is different
  G4OpticalSurface * testSurf;
  G4OpticalSurface * opSurf_silicon;      // silicon (bulk of SiPM) using dielectric_metal
  G4OpticalSurface * opSurf_ceramic;      // little diffuse reflection

  G4OpticalSurface * opSurf_myCorrector1_diffuserIn;  // my own optical surface for doing the PDE correction. applies diffusion in
  G4OpticalSurface * opSurf_myCorrector2_absorptionOut; // another absorptive surface, to scale the G4 reflectivity to match that measured in vacuum

  // longpass filter
  G4OpticalSurface * opSurf_lpFilter;

  // general purpose optical surfaces  
  G4OpticalSurface * opSurf_perfectTrans; // full transmission, no refraction
  G4OpticalSurface * opSurf_perfectRef;   // full specular reflection

};

#endif