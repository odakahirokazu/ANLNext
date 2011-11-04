### CMakeLists.txt for ANL Next

cmake_minimum_required(VERSION 2.8)

project(ANLNext)

find_package( Boost 1.47.0 )

# Definition of installation path
set(AN_INSTALL_PREFIX $ENV{HOME})
set(AN_BIN_INSTALL_PATH ${AN_INSTALL_PREFIX}/bin)
set(AN_LIB_INSTALL_PATH ${AN_INSTALL_PREFIX}/lib)

message("\"make install\" will install executables and dynamic libraries to ${AN_BIN_INSTALL_PATH} and ${AN_LIB_INSTALL_PATH}")

set(AN_TOP ${CMAKE_CURRENT_SOURCE_DIR})
set(AN_TOP_BUILD ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_MODULE_PATH ${AN_TOP}/cmake ${CMAKE_MODULE_PATH})

option(USE_TVector "enable ROOT vector" OFF)
option(USE_HepVector "enable CLHEP vector" OFF)
option(USE_Ruby "enable Ruby binding" ON)

# build type
set(CMAKE_BUILD_TYPE Release)

# compile option (Debug) 
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_DEBUG "-g")

# compile option (Release)
set(CMAKE_CXX_FLAGS_RELEASE "-W -O2")
set(CMAKE_C_FLAGS_RELEASE "-W -O2")
# set(CMAKE_CXX_FLAGS_RELEASE "-W -Wall -O2")
# set(CMAKE_C_FLAGS_RELEASE "-W -Wall -O2")

add_subdirectory(source)
