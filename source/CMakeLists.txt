####### definition of the target
set(TARGET_LIBRARY ANLNext)
#######

set(READLINE_LIB readline)
set(BOOST_INC_DIR ${Boost_INCLUDE_DIRS})

if(ANL_USE_TVECTOR)
  set(ROOTSYS $ENV{ROOTSYS})
  # set(ROOTDIR "/root")
  set(ROOTDIR "")
  set(ROOT_INC_DIR ${ROOTSYS}/include${ROOTDIR})
  set(ROOT_LIB_DIR ${ROOTSYS}/lib${ROOTDIR})
  set(ROOT_LIB Core Physics)
  add_definitions(-DANL_USE_TVECTOR)
endif(ANL_USE_TVECTOR)

if(ANL_USE_HEPVECTOR)
  ### use system-CLHEP
  add_definitions(-DANL_USE_HEPVECTOR)
  set(CLHEP_BASE_DIR $ENV{CLHEP_BASE_DIR})
  set(CLHEP_INC_DIR ${CLHEP_BASE_DIR}/include)
  set(CLHEP_LIB_DIR ${CLHEP_BASE_DIR}/lib)
  set(CLHEP_LIB CLHEP)
endif(ANL_USE_HEPVECTOR)

if(ANL_USE_G4VECTOR)
  ### use Geant4-CLHEP (Geant4.9.5 or later)
  add_definitions(-DANL_USE_HEPVECTOR)
  find_package(Geant4 REQUIRED)
  include(${Geant4_USE_FILE})
  set(CLHEP_INC_DIR ${Geant4_INCLUDE_DIRS})
  set(CLHEP_LIB G4clhep)
endif(ANL_USE_G4VECTOR)

include_directories(
  include
  ${CLHEP_INC_DIR}
  ${ROOT_INC_DIR}
  ${BOOST_INC_DIR}
  )

link_directories(${ROOT_LIB_DIR} ${CLHEP_LIB_DIR})

add_library(${TARGET_LIBRARY} SHARED
  src/ANLManager.cc
  src/ANLManager_interactive.cc
  src/BasicModule.cc
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

if(ANL_INSTALL_HEADERS)
  install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/anl
    FILES_MATCHING PATTERN "*.hh")
endif(ANL_INSTALL_HEADERS)

if(ANL_USE_RUBY)
  add_subdirectory(ruby)
  install(FILES script/ANLLib.rb
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/ruby)
endif(ANL_USE_RUBY)
