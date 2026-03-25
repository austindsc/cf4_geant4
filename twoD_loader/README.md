# Simple 2D Lookup Table Library

**Keep It Simple!** This library loads CSV data into ROOT TH2D objects for 2D interpolation. That's it.

## What it does

1. **Load CSV** → ROOT TH2D histogram
2. **Sample values** with bilinear interpolation  
3. **Handle complex numbers** (extracts real part)

## Files

```
twoD_loader/
├── include/TwoDLookupTable.hh      # Core class
├── src/TwoDLookupTable.cc          # Implementation  
├── test_2d_loader.cc               # Simple test
├── CMakeLists.txt                  # Build config
├── dataPDE/                        # Your data files
└── README.md                       # This file
```

## Usage

```cpp
#include "TwoDLookupTable.hh"

// Load data
TwoDLookupTable table("my_table");
table.LoadFromCSV("data.csv", 0, 1, 2);  // x_col, y_col, z_col

// Sample
double value = table.Sample(wavelength, angle, true); // interpolate=true
```

## Build & Test

```bash
mkdir build
cd build
cmake ..
make
./test_2d_loader
```

## CSV Format

Works with:
- **Simple**: `wavelength,angle,value`
- **Complex**: `wavelength,angle,(1.23+4.56j)` (extracts real part)

## That's it!

No managers, no helpers, no complexity. Just CSV → TH2D → interpolation.

## File Structure

```
twoD_loader/
├── include/
│   ├── TwoDLookupTable.hh       # Core 2D lookup table class
│   ├── SiPMOpticsManager.hh     # Manager for multiple tables
│   └── SiPMOpticsHelpers.hh     # Standalone utility functions
├── src/
│   ├── TwoDLookupTable.cc
│   └── SiPMOpticsManager.cc
├── test_standalone.cc           # Standalone test program
├── CMakeLists.txt               # Build configuration (standalone)
├── dataOptics/                  # Sample data files
│   ├── sample_reflectivity.csv
│   └── sample_transmission.csv
└── README.md                    # This file
```

## CSV File Format

Your CSV files should have the following format:

```csv
# Comments start with #
wavelength,angle,property_value
200,0,0.65
200,15,0.66
200,30,0.68
...
```

Where:
- **wavelength**: Wavelength in nanometers
- **angle**: Incident angle in degrees
- **property_value**: Reflectivity/transmission fraction [0-1]

## Basic Usage

### 1. Loading Data in Your Geant4 Code

```cpp
// In your detector construction or sensitive detector initialization
#include "SiPMOpticsManager.hh"

// Get the manager instance
SiPMOpticsManager* opticsManager = SiPMOpticsManager::GetInstance();

// Load reflectivity and transmission tables
opticsManager->LoadReflectivityTable("dataOptics/your_reflectivity.csv");
opticsManager->LoadTransmissionTable("dataOptics/your_transmission.csv");

// Enable 2D lookup tables in your SensitiveSIPM
// (This is now handled automatically when tables are loaded)
```

### 2. Using in SensitiveSIPM

The `SensitiveSIPM` class has been updated to automatically use the 2D lookup tables when available:

```cpp
// In your sensitive detector construction
SensitiveSIPM* sipmDetector = new SensitiveSIPM("SiPM");

// Load the lookup tables
sipmDetector->LoadOpticsLookupTables("dataOptics/reflectivity.csv", 
                                    "dataOptics/transmission.csv");

// The detector will now automatically use 2D lookup tables for surface interactions
```

### 3. Manual Sampling (Advanced)

```cpp
#include "SiPMOpticsManager.hh"

SiPMOpticsManager* manager = SiPMOpticsManager::GetInstance();

double wavelength = 350.0; // nm
double angle = 30.0;       // degrees

double reflectivity = manager->SampleReflectivity(wavelength, angle);
double transmission = manager->SampleTransmission(wavelength, angle);
```

## Building

The library is automatically built as part of your main Geant4 project. The main CMakeLists.txt has been updated to:

1. Build the TwoDLoader as a static library
2. Link it with your main executable
3. Include the necessary headers

### Building the Test Program

To build and run the standalone test:

```bash
cd twoD_loader
g++ -o test_2d_loader test_2d_loader.cc \
    -I./include \
    -I$(root-config --incdir) \
    $(root-config --libs) \
    src/TwoDLookupTable.cc src/SiPMOpticsManager.cc

./test_2d_loader
```

## Integration with Your SiPM Model

The system is designed to work with your existing SiPM model with minimal changes:

1. **Automatic Detection**: When lookup tables are loaded, the `SensitiveSIPM` automatically switches to using them for surface interactions
2. **Fallback**: If tables aren't loaded, it falls back to your original behavior or Fresnel calculations
3. **Flexible**: You can enable/disable 2D lookup tables at runtime

### Current Implementation Status

- ✅ **Surface Detection**: Photons at SiPM surfaces are detected
- ✅ **Angle Calculation**: Incident angles are calculated from photon momentum and surface normal
- ✅ **Wavelength Calculation**: Wavelengths are calculated from photon momentum
- ✅ **Lookup Table Sampling**: Values are interpolated from 2D tables
- ✅ **Reflection/Transmission Decision**: Random decisions based on lookup table values
- 🚧 **Surface Reflection**: Currently kills reflected photons (you may want to implement actual reflection)

### Next Steps for Full Implementation

To make your reflection/transmission fully realistic, you may want to:

1. **Implement Actual Reflection**: Instead of killing reflected photons, calculate the new momentum direction and continue tracking
2. **Surface Normal Calculation**: Adapt `SiPMOpticsHelpers::GetSiPMSurfaceNormal()` to match your actual geometry
3. **Multiple Surface Types**: Support different lookup tables for different SiPM surface types or coatings

## Data Requirements

Replace the sample data files with your actual measurements:
- `dataOptics/sample_reflectivity.csv` → your reflectivity data
- `dataOptics/sample_transmission.csv` → your transmission data

The sample files contain realistic-looking data but are not based on actual measurements.

## Troubleshooting

### Common Issues

1. **"Table not loaded" errors**: Make sure CSV files exist and are properly formatted
2. **Compilation errors**: Ensure ROOT is properly installed and found by CMake
3. **Interpolation warnings**: Check that your data covers the wavelength/angle ranges you're simulating

### Debug Mode

Enable verbose output by setting the verbosity level in your Geant4 code:
```cpp
// This will show detailed information about photon interactions
// when verbosity >= 2
```

## Performance Notes

- **Interpolation**: ROOT's TH2D::Interpolate() provides bilinear interpolation for smooth results
- **Memory**: Tables are loaded once and kept in memory for fast access
- **Singleton**: SiPMOpticsManager uses singleton pattern to avoid multiple instances

## Extending the System

### Adding Custom Tables

```cpp
// Load a custom table (e.g., quantum efficiency vs wavelength and bias voltage)
manager->LoadCustomTable("quantum_efficiency", "dataOptics/qe_vs_wavelength_voltage.csv");

// Sample from it
double qe = manager->SampleCustomTable("quantum_efficiency", wavelength, voltage);
```

### Different CSV Formats

The loader supports different CSV formats by specifying column indices:
```cpp
// If your CSV has format: angle,wavelength,reflectivity
manager->LoadReflectivityTable("data.csv", 1, 0, 2); // x=wavelength(col1), y=angle(col0), z=refl(col2)
```
