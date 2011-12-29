cmake_minimum_required(VERSION 2.8)

project(MyPackage)

set(MY_LIBRARY MyPackage)

### Definition of installation path
set(AN_INSTALL_PREFIX $ENV{HOME})
set(AN_BIN_INSTALL_PATH ${AN_INSTALL_PREFIX}/bin)
set(AN_LIB_INSTALL_PATH ${AN_INSTALL_PREFIX}/lib)

message("\"make install\" will install executables and dynamic libraries to ${AN_BIN_INSTALL_PATH} and ${AN_LIB_INSTALL_PATH}")

set(ANLNEXT_DIR $ENV{ANLNEXT_TOP}/source)
# set(CMAKE_MODULE_PATH cmake ${CMAKE_MODULE_PATH})

option(USE_Ruby "enable Ruby binding" ON)
# option(USE_Exe "enable standalone executables" OFF)

### build type
set(CMAKE_BUILD_TYPE Release)

### compile option (Debug) 
set(CMAKE_CXX_FLAGS_DEBUG "-g -W -Wall")
set(CMAKE_C_FLAGS_DEBUG "-g -W -Wall")

### compile option (Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -W -Wall")
set(CMAKE_C_FLAGS_RELEASE "-O2 -W -Wall")

# add_definitions(-DANL_USE_TVECTOR -DANL_USE_HEPVECTOR)

include_directories(
  include
  ${ANLNEXT_DIR}/include
  )

link_directories(
  ${AN_LIB_INSTALL_PATH}
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

install(TARGETS ${MY_LIBRARY} LIBRARY DESTINATION ${AN_LIB_INSTALL_PATH})

# if(USE_Exe)
#   add_subdirectory(executable)
# endif(USE_Exe)

if(USE_Ruby)
  add_subdirectory(ruby)
endif(USE_Ruby)
