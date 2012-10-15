cmake_minimum_required(VERSION 2.8)

### Initial definition of cmake variables
set(CMAKE_INSTALL_PREFIX $ENV{HOME} CACHE PATH "install prefix")
set(CMAKE_BUILD_TYPE Release CACHE STRING "build type")
set(CMAKE_CXX_FLAGS_DEBUG "-g -W -Wall" CACHE STRING "CXX_FLAGS for debug")
set(CMAKE_C_FLAGS_DEBUG "-g -W -Wall" CACHE STRING "C_FLAGS for debug")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -W -Wall" CACHE STRING "CXX_FLAGS for release")
set(CMAKE_C_FLAGS_RELEASE "-O3 -W -Wall" CACHE STRING "C_FLAGS for release")

### Definition of project
project(MyPackage)

set(MY_LIBRARY MyPackage)
set(USE_RUBY ON)

### External libraries
### BOOST ###
find_package(Boost 1.48.0 REQUIRED)
set(BOOST_INC_DIR ${Boost_INCLUDE_DIRS})
set(BOOST_LIB_DIR ${Boost_LIBRARY_DIRS})
if(NOT (Boost_VERSION VERSION_LESS 105000))
  find_package(Boost 1.50.0 COMPONENTS system REQUIRED)
  if(Boost_system_LIBRARY)
    set(BOOST_LIB ${Boost_system_LIBRARY})
  else(Boost_system_LIBRARY)
    if(Boost_USE_MULTITHREADED)
      set(BOOST_LIB boost_system-mt)
    else()
      set(BOOST_LIB boost_system)
    endif()
  endif(Boost_system_LIBRARY)
endif()

### ANL ###
if(NOT DEFINED ANLNEXT_INSTALL)
  if($ENV{ANLNEXT_INSTALL})
    set(ANLNEXT_INSTALL $ENV{ANLNEXT_INSTALL})
  else($ENV{ANLNEXT_INSTALL})
    set(ANLNEXT_INSTALL $ENV{HOME})
  endif($ENV{ANLNEXT_INSTALL})
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
  )

link_directories(
  ${ANLNEXT_LIB_DIR}
  )

set(ANL_MODULES
  src/MyModule.cc
  )

add_library(${MY_LIBRARY} SHARED
  ${ANL_MODULES}
)

target_link_libraries(${MY_LIBRARY}
  ANLNext
  )

install(TARGETS ${MY_LIBRARY} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

if(USE_RUBY)
  add_subdirectory(ruby)
endif(USE_RUBY)
