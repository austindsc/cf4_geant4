#include "TwoDLookupTable.hh"
#include <iostream>

/**
 * Simple test: Load HPK-VUV4 file, create transmission and reflection tables, sample them
 */

int main() {
    std::cout << "=== Simple 2D Lookup Table Test ===" << std::endl;
    
    const std::string filename = "dataPDE/optics_HPK-VUV4_wWindow_17nmOxide_defaultData_globalPar_160k.csv";
    
    //////// test 1 ////////
    // Create transmission table (column 2 = transmission_vacuum)
    std::cout << "\n1. Loading transmission data..." << std::endl;
    TwoDLookupTable transmissionTable("transmission");
    bool success1 = transmissionTable.LoadFromCSV(filename, 0, 1, 2, true, ',');
    if (!success1) {
        std::cout << "❌ Failed to load transmission data" << std::endl;
        return 1;
    }
    std::cout << "✅ Transmission table loaded" << std::endl;
    
    //////// test 2 ////////
    // Create reflection table (column 4 = reflection_vacuum)  
    std::cout << "\n2. Loading reflection data..." << std::endl;
    TwoDLookupTable reflectionTable("reflection");
    bool success2 = reflectionTable.LoadFromCSV(filename, 0, 1, 4, true, ',');
    if (!success2) {
        std::cout << "❌ Failed to load reflection data" << std::endl;
        return 1;
    }
    std::cout << "✅ Reflection table loaded" << std::endl;

    ////// sample both tables ///////
    std::cout << "\n3. Sampling tables..." << std::endl;
    double testWavelength = 350.0; // nm
    double testAngle = 30.0;       // degrees
    
    double transmission = transmissionTable.Sample(testWavelength, testAngle, true);
    double reflection = reflectionTable.Sample(testWavelength, testAngle, true);

    // try getting raw histograms
    const TH2D* transHist = transmissionTable.GetHistogram();
    double trans_hist_sample = transHist->Interpolate(testWavelength, testAngle);
    
    std::cout << "Results at λ=" << testWavelength << "nm, θ=" << testAngle << "°:" << std::endl;
    std::cout << "  Transmission: " << transmission << std::endl;
    std::cout << "  (Histogram sample: " << trans_hist_sample << ")" << std::endl;
    std::cout << "  Reflection:   " << reflection << std::endl;
    std::cout << "  Sum:          " << (transmission + reflection) << std::endl;
    
    std::cout << "\n✅ Test complete!" << std::endl;
    return 0;
}
