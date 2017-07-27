####### CMakeLists.txt for ANL Next
cmake_minimum_required(VERSION 3.8)

### Initial definition of cmake variables
set(CMAKE_INSTALL_PREFIX $ENV{HOME} CACHE PATH "install prefix")
set(CMAKE_BUILD_TYPE Release CACHE STRING "build type")
set(CMAKE_CXX_FLAGS_DEBUG "-g -W -Wall" CACHE STRING "CXX_FLAGS for debug")
set(CMAKE_C_FLAGS_DEBUG "-g -W -Wall" CACHE STRING "C_FLAGS for debug")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -W -Wall" CACHE STRING "CXX_FLAGS for release")
set(CMAKE_C_FLAGS_RELEASE "-O3 -W -Wall" CACHE STRING "C_FLAGS for release")
set(CMAKE_MACOSX_RPATH 1)

### Definition of project
project(ANLNext)
add_definitions("-std=c++11")
set(CMAKE_MODULE_PATH ${ANLNext_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

message("-- Install destination prefix: ${CMAKE_INSTALL_PREFIX}")
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

### options
## mode options
option(ANLNEXT_ENABLE_INTERACTIVE_MODE "enable interactive mode" ON)
## library options
option(ANLNEXT_USE_READLINE "enable readline" ON)
option(ANLNEXT_USE_TVECTOR "enable ROOT vector" OFF)
option(ANLNEXT_USE_HEPVECTOR "enable CLHEP vector" OFF)
option(ANLNEXT_USE_G4VECTOR "enable Geant4-CLHEP vector" OFF)
## install options
option(ANLNEXT_INSTALL_HEADERS "install all header files" ON)
option(ANLNEXT_INSTALL_CMAKE_FILES "install all cmake files" ON)
## shortcut options
option(ANLNEXT_USE_ALL "use all libraries" OFF)

if(ANLNEXT_USE_ALL)
  set(ANLNEXT_USE_READLINE ON)
  set(ANLNEXT_USE_TVECTOR ON)
  set(ANLNEXT_USE_G4VECTOR ON)
endif(ANLNEXT_USE_ALL)

### subdirecties
add_subdirectory(source)
add_subdirectory(cmake)

### END
