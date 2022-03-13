#include <iostream>
#include <string>
#include <filesystem>
#include <unistd.h>
#include </usr/local/include/gsl/gsl_math.h>

int main() {
    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
    
    std::cout << "yooo" << std::endl;
    return EXIT_SUCCESS;
}