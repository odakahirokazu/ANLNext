####### definition of the target
add_library(ANLNext)
#######

### Interactive mode
if(ANLNEXT_ENABLE_INTERACTIVE_MODE)
  target_compile_definitions(ANLNext
    PRIVATE
    ANLNEXT_ENABLE_INTERACTIVE_MODE
  )
endif(ANLNEXT_ENABLE_INTERACTIVE_MODE)

### BOOST
find_package(Boost CONFIG 1.80.0)
set(BOOST_INC_DIR ${Boost_INCLUDE_DIRS})
set(BOOST_LIB_DIR ${Boost_LIBRARY_DIRS})
set(BOOST_LIB ${Boost_LIBRARIES})
message("-- BOOST_INC_DIR: ${BOOST_INC_DIR}")
message("-- BOOST_LIB_DIR: ${BOOST_LIB_DIR}")
message("-- BOOST_LIB: ${BOOST_LIB}")

### READLINE
if(ANLNEXT_USE_READLINE)
  set(READLINE_LIB readline)
  target_compile_definitions(ANLNext
    PRIVATE
    ANLNEXT_USE_READLINE
  )
else(ANLNEXT_USE_READLINE)
  set(READLINE_LIB edit)
endif(ANLNEXT_USE_READLINE)

### ROOT
if(ANLNEXT_USE_TVECTOR)
  set(ROOTSYS $ENV{ROOTSYS})
  list(APPEND CMAKE_PREFIX_PATH ${ROOTSYS})
  find_package(ROOT REQUIRED COMPONENTS Core Physics)
  set(ROOT_INC_DIR ${ROOT_INCLUDE_DIRS})
  set(ROOT_LIB_DIR ${ROOT_LIBRARY_DIR})
  set(ROOT_LIB ${ROOT_LIBRARIES})
  list(APPEND CMAKE_INSTALL_RPATH ${ROOT_LIBRARY_DIR})
  message("-- ROOTSYS = ${ROOTSYS}")
  message("-- ROOT_INC_DIR = ${ROOT_INC_DIR}")
  message("-- ROOT_LIB_DIR = ${ROOT_LIB_DIR}")
  message("-- ROOT libraries = ${ROOT_LIB}")
  target_compile_definitions(ANLNext
    PRIVATE
    ANLNEXT_USE_TVECTOR
  )
endif(ANLNEXT_USE_TVECTOR)

### CLHEP
if(ANLNEXT_USE_HEPVECTOR)
  ### use system-CLHEP
  target_compile_definitions(ANLNext
    PRIVATE
    ANLNEXT_USE_HEPVECTOR
  )
  set(CLHEP_BASE_DIR $ENV{CLHEP_BASE_DIR})
  set(CLHEP_INC_DIR ${CLHEP_BASE_DIR}/include)
  set(CLHEP_LIB_DIR ${CLHEP_BASE_DIR}/lib)
  set(CLHEP_LIB CLHEP)
endif(ANLNEXT_USE_HEPVECTOR)

### Geant4
if(ANLNEXT_USE_G4VECTOR)
  ### use Geant4-CLHEP (Geant4.9.5 or later)
  target_compile_definitions(ANLNext
    PRIVATE
    ANLNEXT_USE_HEPVECTOR
  )
  find_package(Geant4 REQUIRED)
  set(CLHEP_INC_DIR ${Geant4_INCLUDE_DIRS})
  set(CLHEP_LIB G4clhep)
endif(ANLNEXT_USE_G4VECTOR)

target_sources(ANLNext
  PRIVATE
  src/ANLStatus.cc
  src/ANLException.cc
  src/EvsManager.cc
  src/CLIUtility.cc
  src/VModuleParameter.cc
  src/ModuleAccess.cc
  src/ParameterRegistry.cc
  src/ModuleDescription.cc
  src/EvsInterface.cc
  src/BasicModule.cc
  src/ANLManager.cc
  src/ANLManager_interactive.cc
  src/ParallelChain.cc
  src/ANLManagerMT.cc
)

### CMake main settings
target_include_directories(ANLNext
  PUBLIC
  ${BOOST_INC_DIR}
  ${CLHEP_INC_DIR}
  ${ROOT_INC_DIR}
  ${READLINE_INC_DIR}
  include
  )

target_link_libraries(ANLNext
  PUBLIC
  ${BOOST_LIB}
  ${ROOT_LIB}
  ${CLHEP_LIB}
  ${READLINE_LIB}
  )

install(TARGETS ANLNext
  LIBRARY
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

if(ANLNEXT_INSTALL_HEADERS)
  install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/anlnext
    FILES_MATCHING PATTERN "*.hh"
  )
endif(ANLNEXT_INSTALL_HEADERS)

add_subdirectory(bindings/ruby)
add_subdirectory(bindings/python)
