# mkdir buildName
# cd buildName
# cmake ../myCF4_or_whatever_new_projectNameIs -DUSE_REFPROP=OFF
# ^ if you don't want to build with REFPROP

# see refpropNotes.txt for install instructions relevant to working with this project
# some env variables and data locations are required


#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.8...3.18)
if(${CMAKE_VERSION} VERSION_LESS 3.12)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()
project(projCF4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_FLAGS "-std=c++17 -lstdc++fs") 

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED qt ui_all vis_all)
else()
  find_package(Geant4 REQUIRED qt)
endif()

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
# Setup include directory for this project
#
include(${Geant4_USE_FILE})

## using ROOT ##
find_package(ROOT REQUIRED)


include_directories(${PROJECT_SOURCE_DIR}/include
                    ${ROOT_INCLUDE_DIRS}
                  )

#----------------------------------------------------------------------------
# Add the TwoDLoader library subdirectory
#
add_subdirectory(twoD_loader)

# Include the TwoDLoader headers
include_directories(${TWOD_LOADER_INCLUDE_DIR})

message(STATUS "ROOT include directories: ${ROOT_INCLUDE_DIRS}")


#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
# add_library(OpticalHelper )
# instead this is just a header/source file with a namespace

# handling refprop (in case something goes wrong with the install)
option(USE_REFPROP "Enable use of REFPROP cpp library to get density" ON)
if(USE_REFPROP)
    add_definitions(-DUSE_REFPROP)
endif()

add_executable(cf4Chamber cf4Chamber.cc ${sources} ${headers})
target_link_libraries(cf4Chamber ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} ${TWOD_LOADER_LIBRARY})

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build CF4. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
# set(EXAMPLECF4_SCRIPTS
#   exampleCF4.in
#   exampleCF4.out
#   init_vis.mac
#   run1.mac
#   run2.mac
#   AtownRun.mac
#   myVis.mac
#   vis.mac
#   )

# foreach(_script ${EXAMPLECF4_SCRIPTS})
#   configure_file(
#     ${PROJECT_SOURCE_DIR}/${_script}
#     ${PROJECT_BINARY_DIR}/${_script}
#     COPYONLY
#     )
# endforeach()

file(GLOB DATA_FOLDER "data")
file(COPY ${DATA_FOLDER} DESTINATION ${PROJECT_BINARY_DIR})
message(STATUS "copied ${DATA_FOLDER} folder to build directory")

file(GLOB DATA_FOLDER "dataOptics")
file(COPY ${DATA_FOLDER} DESTINATION ${PROJECT_BINARY_DIR})
message(STATUS "copied ${DATA_FOLDER} folder to build directory")

#----------------------------------------------------------------------------
# For internal Geant4 use - but has no effect if you build this
# example standalone
#
add_custom_target(projCF4 DEPENDS cf4)

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS cf4Chamber DESTINATION bin)
