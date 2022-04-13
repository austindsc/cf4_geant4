#ifndef DETECTORMATERIALS_H
#define DETECTORMATERIALS_H 1

#include "globals.hh"
#include "G4SystemOfUnits.hh"

class G4Material;
class G4String;

class DetectorMaterials{
  friend class B1DetectorConstruction;
  friend class MaterialsMessenger;
  friend class collimator;
  friend class alphaSource1;
  
public:
  void BuildMaterials(); //call this after setting variables with messenger

  void SetDetectionMedium(G4String med);
  G4String GetDetectorMediumString(){return fmedString; }
  G4Material * GetDetectorMedium(){return *fDetectorMedium; }
  
  void SetMediumDensity(G4double r){ fmediumDensity = r; }
  G4double GetMediumDensity(){ return fmediumDensity; }
  
  void SetMediumLY(G4double ly){ if(ly == 0/MeV) ly = 0.1/MeV; fmediumLY = ly; }
  G4double GetMediumLY(){ return fmediumLY; }
  
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
  G4Material * mSteel;
  G4Material * mLead;
  G4Material * mGenericPlastic;
  G4Material * mAluminum;
  // G4Material * mBlackPaper;
  
  // SiPM stuff
  G4Material * mSilicon;
  // G4Material * Si02;      //silicon dioxide (for SiPM surface?)
  // G4Material * mQuartz;
  G4Material * mCeramic;
  
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
  
  // and their properties
  G4double fmediumLY;
  G4double fmediumDensity;
  G4double fmediumTemp;
  G4double fmediumPressure;

};

#endif