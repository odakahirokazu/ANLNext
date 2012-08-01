### CMakeLists.txt for ANL Next

cmake_minimum_required(VERSION 2.8)

project(ANLNext)

find_package(Boost 1.48.0 Required)

### Definition of installation path
set(AN_INSTALL_PREFIX $ENV{HOME})
set(AN_BIN_INSTALL_PATH ${AN_INSTALL_PREFIX}/bin)
set(AN_LIB_INSTALL_PATH ${AN_INSTALL_PREFIX}/lib)

message("\"make install\" will install executables and dynamic libraries to ${AN_BIN_INSTALL_PATH} and ${AN_LIB_INSTALL_PATH}")

set(AN_TOP ${CMAKE_CURRENT_SOURCE_DIR})
set(AN_TOP_BUILD ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_MODULE_PATH ${AN_TOP}/cmake ${CMAKE_MODULE_PATH})

option(USE_TVector "enable ROOT vector" OFF)
option(USE_HepVector "enable CLHEP vector" OFF)
option(USE_G4Vector "enable G4-CLHEP vector" OFF)
option(USE_Ruby "enable Ruby binding" ON)
option(USE_All "enable all libraries" OFF)

if(USE_All)
  set(USE_TVector ON)
  set(USE_G4Vector ON)
endif(USE_All)

### build type
set(CMAKE_BUILD_TYPE Release)

### compile option (Debug) 
set(CMAKE_CXX_FLAGS_DEBUG "-g -W -Wall")
set(CMAKE_C_FLAGS_DEBUG "-g -W -Wall")

### compile option (Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -W -Wall")
set(CMAKE_C_FLAGS_RELEASE "-O2 -W -Wall")

add_subdirectory(source)
