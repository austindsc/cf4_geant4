#ifndef DETECTORMATERIALS_H
#define DETECTORMATERIALS_H 1

#include "globals.hh"

class G4Material;
class G4String;

class DetectorMaterials{
  friend class B1DetectorConstruction;
  friend class MaterialsMessenger;
public:
  DetectorMaterials();
  virtual ~DetectorMaterials();
  
  void BuildMaterials(); //call this after setting variables with messenger

  void SetDetectionMedium(G4String med);
  G4String GetDetectorMediumString(){return fmedString; }
  G4Material * GetDetectorMedium(){return *fDetectorMedium; }
  
  void SetMediumDensity(G4double r){ fmediumDensity = r; }
  G4double GetMediumDensity(){ return fmediumDensity; }
  
  void SetMediumLY(G4double ly){ fmediumLY = ly; }
  G4double GetMediumLY(){ return fmediumLY; }
  
  void SetMediumTemp(G4double temp){ fmediumTemp = temp; }
  G4double GetMediumTemp(){ return fmediumTemp; }
  
private:
  // chamber materials
  G4Material * mVacuum; //vacuum
  G4Material * mWalls; // black absorptive paper stuff  
  G4Material * mSteel;
  G4Material * mLead;

  // different mediums
  G4Material * mGasCF4;
  G4Material * mLiquidCF4;
  G4Material * mGasAr;
  
  G4Material * mSilicon;
  G4Material * Si02; //silicon dioxide (for SiPM surface?)
  G4Material * mQuartz;
  
  // smoke detector (alpha source)
  G4Material * mAmActiveMatrix; // gold mixed with AmO2
  G4Material * mSilver;
  G4Material * mGold;
  G4Material * mPalladium;
  
  // for use by the materials messenger
  G4String fmedString;
  G4Material ** fDetectorMedium;// what is currently in active volume
  
  G4double fmediumLY;
  G4double fmediumDensity;
  G4double fmediumTemp;
  G4double fmediumPressure;
};

#endif