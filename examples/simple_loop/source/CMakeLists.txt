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
project(MyPackage)
add_definitions("-std=c++11")

set(MY_LIBRARY MyPackage)
set(USE_RUBY ON)

### External libraries
### BOOST ###
find_package(Boost 1.56.0 REQUIRED COMPONENTS system chrono thread)
set(BOOST_INC_DIR ${Boost_INCLUDE_DIRS})
set(BOOST_LIB_DIR ${Boost_LIBRARY_DIRS})
set(BOOST_LIB ${Boost_LIBRARIES})
message("-- BOOST_INC_DIR: ${BOOST_INC_DIR}")
message("-- BOOST_LIB_DIR: ${BOOST_LIB_DIR}")
message("-- BOOST_LIB: ${BOOST_LIB}")

### ANL ###
if(NOT DEFINED ANLNEXT_INSTALL)
  if(DEFINED ENV{ANLNEXT_INSTALL})
    set(ANLNEXT_INSTALL $ENV{ANLNEXT_INSTALL})
  else()
    set(ANLNEXT_INSTALL $ENV{HOME})
  endif()
endif(NOT DEFINED ANLNEXT_INSTALL)
set(ANLNEXT_INC_DIR ${ANLNEXT_INSTALL}/include/anl)
set(ANLNEXT_LIB_DIR ${ANLNEXT_INSTALL}/lib)
set(ANLNEXT_LIB ANLNext)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ANLNEXT_LIB_DIR}/anl)
message("-- ANLNEXT_INSTALL = ${ANLNEXT_INSTALL}")

# add_definitions(-DANL_USE_TVECTOR -DANL_USE_HEPVECTOR)

include_directories(
  include
  ${ANLNEXT_INC_DIR}
  ${BOOST_INC_DIR}
  )

link_directories(
  ${ANLNEXT_LIB_DIR}
  ${BOOST_LIB_DIR}
  )

set(ANL_MODULES
  src/MyModule.cc
  src/MyVectorModule.cc
  src/MyMapModule.cc
  )

add_library(${MY_LIBRARY} SHARED
  ${ANL_MODULES}
)

target_link_libraries(${MY_LIBRARY}
  ${BOOST_LIB}
  ANLNext
  )

install(TARGETS ${MY_LIBRARY} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

if(USE_RUBY)
  add_subdirectory(rubyext)
endif(USE_RUBY)
