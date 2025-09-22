#include "TwoDLookupTable.hh"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

TwoDLookupTable::TwoDLookupTable(const std::string& name) 
    : fName(name.empty() ? "TwoDLookupTable" : name), 
      fHistogram(nullptr), 
      fIsLoaded(false),
      fXUnit(""), fYUnit(""), fZUnit("") {
}

TwoDLookupTable::~TwoDLookupTable() {
    // std::unique_ptr will handle cleanup automatically
}

bool TwoDLookupTable::LoadFromCSV(const std::string& csvPath, 
                                  int xColumn, int yColumn, int zColumn,
                                  bool hasHeader, char delimiter) {
    return LoadFromCSVComplex(csvPath, xColumn, yColumn, zColumn, hasHeader, delimiter, true);
}

bool TwoDLookupTable::LoadFromCSVComplex(const std::string& csvPath, 
                                        int xColumn, int yColumn, int zColumn,
                                        bool hasHeader, char delimiter, bool useRealPart) {
    
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        std::cerr << "TwoDLookupTable::LoadFromCSV ERROR: Cannot open file: " << csvPath << std::endl;
        return false;
    }

    std::vector<double> xVals, yVals, zVals;
    std::string line;
    int lineNumber = 0;

    // Skip header if present
    if (hasHeader && std::getline(file, line)) {
        lineNumber++;
        std::cout << "TwoDLookupTable::LoadFromCSV: Skipping header: " << line << std::endl;
    }

    // Read data
    while (std::getline(file, line)) {
        lineNumber++;
        if (line.empty() || line[0] == '#') continue; // Skip empty lines and comments

        std::vector<std::string> tokens = ParseCSVLine(line, delimiter);
        
        // Check if we have enough columns
        int maxCol = std::max({xColumn, yColumn, zColumn});
        if (static_cast<int>(tokens.size()) <= maxCol) {
            std::cerr << "TwoDLookupTable::LoadFromCSV WARNING: Line " << lineNumber 
                     << " has insufficient columns (" << tokens.size() << " vs required " 
                     << maxCol + 1 << "): " << line << std::endl;
            continue;
        }

        try {
            double x = std::stod(tokens[xColumn]);
            double y = std::stod(tokens[yColumn]);
            double z;
            
            // Check if the z value might be a complex number
            std::string zToken = tokens[zColumn];
            if (zToken.find('(') != std::string::npos && (zToken.find('j') != std::string::npos || zToken.find('i') != std::string::npos)) {
                // Parse as complex number
                z = ParseComplexNumber(zToken, useRealPart);
            } else {
                // Parse as regular double
                z = std::stod(zToken);
            }
            
            xVals.push_back(x);
            yVals.push_back(y);
            zVals.push_back(z);
        } catch (const std::exception& e) {
            std::cerr << "TwoDLookupTable::LoadFromCSV ERROR: Cannot parse line " 
                     << lineNumber << ": " << line << " (" << e.what() << ")" << std::endl;
            continue;
        }
    }

    file.close();

    if (xVals.empty()) {
        std::cerr << "TwoDLookupTable::LoadFromCSV ERROR: No valid data found in " << csvPath << std::endl;
        return false;
    }

    std::cout << "TwoDLookupTable::LoadFromCSV: Read " << xVals.size() << " data points from " << csvPath << std::endl;

    // Find unique X and Y values to determine binning
    std::vector<double> uniqueX = xVals, uniqueY = yVals;
    std::sort(uniqueX.begin(), uniqueX.end());
    std::sort(uniqueY.begin(), uniqueY.end());
    uniqueX.erase(std::unique(uniqueX.begin(), uniqueX.end()), uniqueX.end());
    uniqueY.erase(std::unique(uniqueY.begin(), uniqueY.end()), uniqueY.end());

    int nX = uniqueX.size();
    int nY = uniqueY.size();

    std::cout << "TwoDLookupTable::LoadFromCSV: Found " << nX << " unique X values, " 
              << nY << " unique Y values" << std::endl;
    std::cout << "X range: [" << uniqueX.front() << ", " << uniqueX.back() << "]" << std::endl;
    std::cout << "Y range: [" << uniqueY.front() << ", " << uniqueY.back() << "]" << std::endl;

    // Create histogram with appropriate binning
    double xMin = uniqueX.front() - (uniqueX.size() > 1 ? (uniqueX[1] - uniqueX[0]) * 0.5 : 0.5);
    double xMax = uniqueX.back() + (uniqueX.size() > 1 ? (uniqueX[nX-1] - uniqueX[nX-2]) * 0.5 : 0.5);
    double yMin = uniqueY.front() - (uniqueY.size() > 1 ? (uniqueY[1] - uniqueY[0]) * 0.5 : 0.5);
    double yMax = uniqueY.back() + (uniqueY.size() > 1 ? (uniqueY[nY-1] - uniqueY[nY-2]) * 0.5 : 0.5);

    TString histName = TString::Format("%s_hist", fName.c_str());
    fHistogram = std::make_unique<TH2D>(histName, histName, nX, xMin, xMax, nY, yMin, yMax);

    // Fill the histogram
    for (size_t i = 0; i < xVals.size(); ++i) {
        int binX = fHistogram->GetXaxis()->FindBin(xVals[i]);
        int binY = fHistogram->GetYaxis()->FindBin(yVals[i]);
        fHistogram->SetBinContent(binX, binY, zVals[i]);
    }

    fIsLoaded = true;
    std::cout << "TwoDLookupTable::LoadFromCSV: Successfully loaded " << fName << std::endl;
    return true;
}

bool TwoDLookupTable::LoadFromArrays(const std::vector<double>& xVals,
                                     const std::vector<double>& yVals,
                                     const std::vector<double>& zVals,
                                     int nX, int nY) {
    
    if (xVals.size() != yVals.size() || xVals.size() != zVals.size()) {
        std::cerr << "TwoDLookupTable::LoadFromArrays ERROR: Array size mismatch" << std::endl;
        return false;
    }

    if (static_cast<int>(xVals.size()) != nX * nY) {
        std::cerr << "TwoDLookupTable::LoadFromArrays ERROR: Array size doesn't match grid dimensions" << std::endl;
        return false;
    }

    auto xMinMax = std::minmax_element(xVals.begin(), xVals.end());
    auto yMinMax = std::minmax_element(yVals.begin(), yVals.end());

    double xMin = *xMinMax.first;
    double xMax = *xMinMax.second;
    double yMin = *yMinMax.first;
    double yMax = *yMinMax.second;

    TString histName = TString::Format("%s_hist", fName.c_str());
    fHistogram = std::make_unique<TH2D>(histName, histName, nX, xMin, xMax, nY, yMin, yMax);

    for (size_t i = 0; i < xVals.size(); ++i) {
        int binX = fHistogram->GetXaxis()->FindBin(xVals[i]);
        int binY = fHistogram->GetYaxis()->FindBin(yVals[i]);
        fHistogram->SetBinContent(binX, binY, zVals[i]);
    }

    fIsLoaded = true;
    std::cout << "TwoDLookupTable::LoadFromArrays: Successfully loaded " << fName << std::endl;
    return true;
}

double TwoDLookupTable::Sample(double x, double y, bool interpolate) const {
    if (!fIsLoaded || !fHistogram) {
        std::cerr << "TwoDLookupTable::Sample ERROR: Table not loaded" << std::endl;
        return 0.0;
    }

    if (interpolate) {
        return fHistogram->Interpolate(x, y);
    } else {
        int binX = fHistogram->GetXaxis()->FindBin(x);
        int binY = fHistogram->GetYaxis()->FindBin(y);
        return fHistogram->GetBinContent(binX, binY);
    }
}

void TwoDLookupTable::SetUnits(const std::string& xUnit, const std::string& yUnit, const std::string& zUnit) {
    fXUnit = xUnit;
    fYUnit = yUnit;
    fZUnit = zUnit;
}

void TwoDLookupTable::PrintInfo() const {
    std::cout << "=== TwoDLookupTable: " << fName << " ===" << std::endl;
    std::cout << "Loaded: " << (fIsLoaded ? "Yes" : "No") << std::endl;
    
    if (fIsLoaded && fHistogram) {
        std::cout << "X bins: " << fHistogram->GetNbinsX() 
                 << " [" << fHistogram->GetXaxis()->GetXmin() << ", " 
                 << fHistogram->GetXaxis()->GetXmax() << "]";
        if (!fXUnit.empty()) std::cout << " " << fXUnit;
        std::cout << std::endl;
        
        std::cout << "Y bins: " << fHistogram->GetNbinsY() 
                 << " [" << fHistogram->GetYaxis()->GetXmin() << ", " 
                 << fHistogram->GetYaxis()->GetXmax() << "]";
        if (!fYUnit.empty()) std::cout << " " << fYUnit;
        std::cout << std::endl;
        
        std::cout << "Z range: [" << fHistogram->GetMinimum() << ", " 
                 << fHistogram->GetMaximum() << "]";
        if (!fZUnit.empty()) std::cout << " " << fZUnit;
        std::cout << std::endl;
    }
    std::cout << "=========================" << std::endl;
}

std::pair<double, double> TwoDLookupTable::GetXRange() const {
    if (!fIsLoaded || !fHistogram) {
        return {0.0, 0.0};
    }
    return {fHistogram->GetXaxis()->GetXmin(), fHistogram->GetXaxis()->GetXmax()};
}

std::pair<double, double> TwoDLookupTable::GetYRange() const {
    if (!fIsLoaded || !fHistogram) {
        return {0.0, 0.0};
    }
    return {fHistogram->GetYaxis()->GetXmin(), fHistogram->GetYaxis()->GetXmax()};
}

std::vector<std::string> TwoDLookupTable::ParseCSVLine(const std::string& line, char delimiter) const {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        // Trim whitespace
        token.erase(0, token.find_first_not_of(" \t\r\n"));
        token.erase(token.find_last_not_of(" \t\r\n") + 1);
        tokens.push_back(token);
    }
    
    return tokens;
}

double TwoDLookupTable::ParseComplexNumber(const std::string& complexStr, bool useRealPart) const {
    // Parse strings like "(1.23+4.56j)" or "(1.23-4.56j)" or "(1.23e-10+4.56e-20j)"
    std::string str = complexStr;
    
    // Remove parentheses and 'j' or 'i'
    str.erase(std::remove(str.begin(), str.end(), '('), str.end());
    str.erase(std::remove(str.begin(), str.end(), ')'), str.end());
    
    // Replace 'j' with 'i' for consistency
    std::replace(str.begin(), str.end(), 'j', 'i');
    
    try {
        // Find the position of '+' or '-' that separates real and imaginary parts
        size_t signPos = std::string::npos;
        
        // Look for the last '+' or '-' that's not part of scientific notation
        for (int i = str.length() - 1; i >= 0; --i) {
            if ((str[i] == '+' || str[i] == '-') && i > 0) {
                // Check if this is not part of scientific notation (e.g., 1.23e-10)
                if (str[i-1] != 'e' && str[i-1] != 'E') {
                    signPos = i;
                    break;
                }
            }
        }
        
        if (signPos != std::string::npos) {
            // Complex number with both real and imaginary parts
            std::string realPart = str.substr(0, signPos);
            std::string imagPart = str.substr(signPos);
            
            // Remove 'i' from imaginary part
            imagPart.erase(std::remove(imagPart.begin(), imagPart.end(), 'i'), imagPart.end());
            
            if (useRealPart) {
                return std::stod(realPart);
            } else {
                return std::stod(imagPart);
            }
        } else {
            // Only real part (no imaginary part found)
            str.erase(std::remove(str.begin(), str.end(), 'i'), str.end());
            return std::stod(str);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "TwoDLookupTable::ParseComplexNumber ERROR: Cannot parse '" 
                 << complexStr << "' (" << e.what() << ")" << std::endl;
        return 0.0;
    }
}
