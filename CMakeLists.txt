####### CMakeLists.txt for ANL Next
cmake_minimum_required(VERSION 2.8)

### Initial definition of cmake variables
set(CMAKE_INSTALL_PREFIX $ENV{HOME} CACHE PATH "install prefix")
set(CMAKE_BUILD_TYPE Release CACHE STRING "build type")
set(CMAKE_CXX_FLAGS_DEBUG "-g -W -Wall" CACHE STRING "CXX_FLAGS for debug")
set(CMAKE_C_FLAGS_DEBUG "-g -W -Wall" CACHE STRING "C_FLAGS for debug")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -W -Wall" CACHE STRING "CXX_FLAGS for release")
set(CMAKE_C_FLAGS_RELEASE "-O3 -W -Wall" CACHE STRING "C_FLAGS for release")
set(CMAKE_MACOSX_RPATH 0)

### Definition of project
project(ANLNext)
add_definitions("-std=c++11")
set(CMAKE_MODULE_PATH ${ANLNext_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

message("-- Install destination prefix: ${CMAKE_INSTALL_PREFIX}")

### options
## mode options
option(ANL_ENABLE_INTERACTIVE_MODE "enable interactive mode" ON)
## library options
option(ANL_USE_READLINE "enable readline" ON)
option(ANL_USE_TVECTOR "enable ROOT vector" OFF)
option(ANL_USE_HEPVECTOR "enable CLHEP vector" OFF)
option(ANL_USE_G4VECTOR "enable Geant4-CLHEP vector" OFF)
## install options
option(ANL_INSTALL_HEADERS "install all header files" ON)
option(ANL_INSTALL_CMAKE_FILES "install all cmake files" ON)
## shortcut options
option(ANL_USE_ALL "use all libraries" OFF)

if(ANL_USE_ALL)
  set(ANL_USE_READLINE ON)
  set(ANL_USE_TVECTOR ON)
  set(ANL_USE_G4VECTOR ON)
endif(ANL_USE_ALL)

### subdirecties
add_subdirectory(source)
add_subdirectory(cmake)

### END
