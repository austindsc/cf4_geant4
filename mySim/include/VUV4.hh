#ifndef VUV4_HH
#define VUV4_HH 1

#include "G4LogicalVolume.hh"

class DetectorMaterials;
class G4SubtractionSolid;

/*
// usage of this class:


*/
class VUV4 : public G4LogicalVolume{
  
public:
  VUV4(G4double zheight = 0, G4bool doOpticalSurfaces = true, G4bool buildWindow = true, G4bool checkOverlaps = false);
  ~VUV4(){}

  static G4SubtractionSolid* Create4Grid(G4double xy, G4double z, G4double middleGap, G4String solid_name = "solid_squares");

  G4LogicalVolume* GetSensitiveVolume() {return fSensitiveSilicon;}

  // ceramic packaging
  static const G4double fPackageThick;
  static const G4double fPackageXY;
  static const G4double fPackageZgap;
  static const G4double fPackageSideGap;
  static const G4double fPackageChannelGap;
  static const G4double fEdgeBuffer;

  // extra medium on top for doing optical layers
  static const G4double fExtraMediumTopThick;

  // silicon square properties
  static const G4double fSipmMiddleGap;
  static const G4double fSipmXY;
  static const G4double fSipmThick;
  static const G4double fSipmSurfaceThick;

  static const G4double fWindowThick;
  static const G4double fWindowFromSurface;

private:
  G4LogicalVolume* fSensitiveSilicon;

};

#endif