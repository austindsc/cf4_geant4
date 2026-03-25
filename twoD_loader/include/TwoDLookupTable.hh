#ifndef TWODLOOKUPTABLE_HH
#define TWODLOOKUPTABLE_HH

#include "TH2D.h"
#include "TString.h"
#include <memory>
#include <string>
#include <vector>

/**
 * @brief A class to handle 2D lookup tables for optical properties
 * 
 * This class loads CSV data and provides interpolated sampling
 * for wavelength and angle-dependent optical properties like
 * reflectivity and transmission, or alternatively for internal PDE
 */
class TwoDLookupTable {
public:
    /**
     * @brief Constructor
     * @param name Name for the lookup table
     */
    explicit TwoDLookupTable(const std::string& name = "");
    
    /**
     * @brief Destructor
     */
    ~TwoDLookupTable();

    /**
     * @brief Load data from CSV file with complex number support
     * @param csvPath Path to CSV file
     * @param xColumn Column index for X values (e.g., wavelength)
     * @param yColumn Column index for Y values (e.g., angle)
     * @param zColumn Column index for Z values (e.g., reflectivity/transmission)
     * @param hasHeader Whether the CSV file has a header row
     * @param delimiter CSV delimiter character
     * @param useRealPart If true, extract real part of complex numbers (default: true)
     * @return true if successful, false otherwise
     */
    bool LoadFromCSVComplex(const std::string& csvPath, 
                           int xColumn = 0, int yColumn = 1, int zColumn = 2,
                           bool hasHeader = true, char delimiter = ',',
                           bool useRealPart = true);

    /**
     * @brief Load data from CSV file
     * @param csvPath Path to CSV file
     * @param xColumn Column index for X values (e.g., wavelength)
     * @param yColumn Column index for Y values (e.g., angle)
     * @param zColumn Column index for Z values (e.g., reflectivity/transmission)
     * @param hasHeader Whether the CSV file has a header row
     * @param delimiter CSV delimiter character
     * @return true if successful, false otherwise
     */
    bool LoadFromCSV(const std::string& csvPath, 
                     int xColumn = 0, int yColumn = 1, int zColumn = 2,
                     bool hasHeader = true, char delimiter = ',');;

    /**
     * @brief Load data from pre-formatted arrays
     * @param xVals X-axis values
     * @param yVals Y-axis values  
     * @param zVals Z values (same size as xVals * yVals)
     * @param nX Number of X bins
     * @param nY Number of Y bins
     */
    bool LoadFromArrays(const std::vector<double>& xVals,
                        const std::vector<double>& yVals,
                        const std::vector<double>& zVals,
                        int nX, int nY);

    /**
     * @brief Sample the lookup table at given coordinates
     * @param x X coordinate (e.g., wavelength)
     * @param y Y coordinate (e.g., angle)
     * @param interpolate Whether to use interpolation (default: true)
     * @return Interpolated/sampled value
     */
    double Sample(double x, double y, bool interpolate = true) const;

    /**
     * @brief Get the underlying ROOT histogram
     * @return Pointer to TH2D histogram
     */
    const TH2D* GetHistogram() const { return fHistogram.get(); }

    /**
     * @brief Check if the table is properly loaded
     * @return true if loaded, false otherwise
     */
    bool IsLoaded() const { return fIsLoaded; }

    /**
     * @brief Get table name
     * @return Table name
     */
    const std::string& GetName() const { return fName; }

    /**
     * @brief Set units for display/debugging
     * @param xUnit X-axis unit (e.g., "nm")
     * @param yUnit Y-axis unit (e.g., "degrees")
     * @param zUnit Z-axis unit (e.g., "fraction")
     */
    void SetUnits(const std::string& xUnit, const std::string& yUnit, const std::string& zUnit);

    /**
     * @brief Print table information
     */
    void PrintInfo() const;

    /**
     * @brief Get X range
     */
    std::pair<double, double> GetXRange() const;
    
    /**
     * @brief Get Y range
     */
    std::pair<double, double> GetYRange() const;

private:
    std::string fName;
    std::unique_ptr<TH2D> fHistogram;
    bool fIsLoaded;
    
    // Units for display
    std::string fXUnit;
    std::string fYUnit; 
    std::string fZUnit;

    /**
     * @brief Parse CSV line into tokens
     */
    std::vector<std::string> ParseCSVLine(const std::string& line, char delimiter) const;
    
    /**
     * @brief Parse complex number string like "(1.23+4.56j)" or "(1.23-4.56j)"
     * @param complexStr String containing complex number
     * @param useRealPart If true, return real part; if false, return imaginary part
     * @return Parsed double value
     */
    double ParseComplexNumber(const std::string& complexStr, bool useRealPart = true) const;
};

#endif // TWODLOOKUPTABLE_HH
