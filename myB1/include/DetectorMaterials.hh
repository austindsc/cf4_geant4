#ifndef DETECTORMATERIALS_H
#define DETECTORMATERIALS_H 1

class G4Material;

class DetectorMaterials{
  friend class B1DetectorConstruction;
public:
  DetectorMaterials();
  virtual ~DetectorMaterials();
  
  void ConstructMaterials(); //call this after setting variables with messenger

  
private:
  // chamber materials
  G4Material * mVacuum; //vacuum
  G4Material * mWalls; // black absorptive paper stuff  
  G4Material * mSteel;
  G4Material * mLead;

  G4Material * mDetectorMedium;// what is currently in active volume
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
};

#endif