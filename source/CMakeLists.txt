####### definition of the target
set(TARGET_LIBRARY ANLNext)
#######

set(READLINE_LIB readline)

if (USE_TVector)
  set(ROOTSYS $ENV{ROOTSYS})
  # set(ROOTDIR "/root")
  set(ROOTDIR "")
  set(ROOT_INC_DIR ${ROOTSYS}/include${ROOTDIR})
  set(ROOT_LIB_DIR ${ROOTSYS}/lib${ROOTDIR})
  set(ROOT_LIB Core Physics)
  add_definitions(-DANL_USE_TVECTOR)
endif (USE_TVector)

if (USE_HepVector)
  ### use system-CLHEP
  add_definitions(-DANL_USE_HEPVECTOR)
  set(CLHEP_BASE_DIR $ENV{CLHEP_BASE_DIR})
  set(CLHEP_INC_DIR ${CLHEP_BASE_DIR}/include)
  set(CLHEP_LIB_DIR ${CLHEP_BASE_DIR}/lib)
  set(CLHEP_LIB CLHEP)
endif (USE_HepVector)

if (USE_G4Vector)
  ### use Geant4-CLHEP (Geant4.9.5 or later)
  add_definitions(-DANL_USE_HEPVECTOR)
  find_package(Geant4 REQUIRED)
  include(${Geant4_USE_FILE})
  set(CLHEP_INC_DIR ${Geant4_INCLUDE_DIRS})
  set(CLHEP_LIB G4clhep)
endif (USE_G4Vector)

include_directories(
  include
  ${CLHEP_INC_DIR}
  ${ROOT_INC_DIR}
  )

link_directories(${ROOT_LIB_DIR} ${CLHEP_LIB_DIR})

add_library(${TARGET_LIBRARY} SHARED
  src/ANLNext.cc
  src/ANLNext_interactive.cc
  src/ANLVModule.cc
  src/ANLException.cc
  src/EvsManager.cc
  src/VModuleParameter.cc
  )

target_link_libraries(${TARGET_LIBRARY}
  ${ROOT_LIB} ${CLHEP_LIB} ${READLINE_LIB})

install(TARGETS ${TARGET_LIBRARY}
  LIBRARY
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/anl
  FILES_MATCHING PATTERN "*.hh")

if(USE_Ruby)
  add_subdirectory(ruby)
  install(FILES script/ANLLib.rb
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/ruby)
endif(USE_Ruby)
