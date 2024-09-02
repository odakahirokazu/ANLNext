cmake_minimum_required(VERSION 3.8)

### Initial definition of cmake variables
set(CMAKE_INSTALL_PREFIX $ENV{HOME} CACHE PATH "install prefix")
set(CMAKE_BUILD_TYPE Release CACHE STRING "build type")
set(CMAKE_CXX_FLAGS_DEBUG "-g -W -Wall" CACHE STRING "CXX_FLAGS for debug")
set(CMAKE_C_FLAGS_DEBUG "-g -W -Wall" CACHE STRING "C_FLAGS for debug")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -W -Wall" CACHE STRING "CXX_FLAGS for release")
set(CMAKE_C_FLAGS_RELEASE "-O3 -W -Wall" CACHE STRING "C_FLAGS for release")
set(CMAKE_MACOSX_RPATH 0)

### Definition of project
project(MyExample)
set(CMAKE_CXX_STANDARD 17)

set(MY_EXE cli_example)

### External libraries
### BOOST ###
find_package(Boost CONFIG 1.80.0)
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
set(ANLNEXT_INC_DIR ${ANLNEXT_INSTALL}/include)
set(ANLNEXT_LIB_DIR ${ANLNEXT_INSTALL}/lib)
set(ANLNEXT_LIB ANLNext)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ANLNEXT_LIB_DIR}/anlnext)
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

set(sources
  cli_example.cc
  )

add_executable(${MY_EXE} ${sources})

target_link_libraries(${MY_EXE}
  ${BOOST_LIB}
  ANLNext
  )

# install(TARGETS ${MY_EXE} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
