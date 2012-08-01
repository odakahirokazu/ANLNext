####### CMakeLists.txt for ANL Next

cmake_minimum_required(VERSION 2.8)

### Initial definition of cmake variables
set(CMAKE_INSTALL_PREFIX $ENV{HOME} CACHE PATH "install prefix")
set(CMAKE_BUILD_TYPE Release CACHE STRING "build type")
set(CMAKE_CXX_FLAGS_DEBUG "-g -W -Wall" CACHE STRING "CXX_FLAGS for debug")
set(CMAKE_C_FLAGS_DEBUG "-g -W -Wall" CACHE STRING "C_FLAGS for debug")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -W -Wall" CACHE STRING "CXX_FLAGS for release")
set(CMAKE_C_FLAGS_RELEASE "-O2 -W -Wall" CACHE STRING "C_FLAGS for release")

### Definition of project
project(ANLNext)
find_package(Boost 1.48.0 REQUIRED)
set(CMAKE_MODULE_PATH ${ANLNext_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

message("-- Install destination prefix: ${CMAKE_INSTALL_PREFIX}")

### options
option(USE_Ruby "enable Ruby binding" ON)
option(USE_Exe "enable standalone executables" OFF)
option(USE_TVector "enable ROOT vector" OFF)
option(USE_HepVector "enable CLHEP vector" OFF)
option(USE_G4Vector "enable G4-CLHEP vector" OFF)
option(USE_ALL "use all libraries" OFF)

if(USE_ALL)
  set(USE_TVector ON)
  set(USE_G4Vector ON)
endif(USE_ALL)

### subdirecties
add_subdirectory(source)
add_subdirectory(cmake)

### END
