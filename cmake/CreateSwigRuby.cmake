### CreateSwigRuby
### 2013-06-02 | Hirokazu Odaka
###

### pre-process
if (NOT DEFINED TARGET_EXT_LIBRARY_OUTPUT)
  set(TARGET_EXT_LIBRARY_OUTPUT ${TARGET_EXT_LIBRARY})
endif()

### message
message("-- CreateSwigRuby: ${TARGET_EXT_LIBRARY}")
message("-- Output name: ${TARGET_EXT_LIBRARY_OUTPUT}")
message("-- SWIG interface file: ${SWIG_IF_FILE}")
message("-- INCLUDE_DIRS: ${RUBY_EXT_INCLUDE_DIRS}")
message("-- LIBRARY_DIRS: ${RUBY_EXT_LIBRARY_DIRS}")
message("-- LIBRARIES: ${RUBY_EXT_LIBRARIES}")
message("-- SWIG_FLAGS: ${SWIG_FLAGS}")
message("-- ADDITIONAL_INSTALL_TARGET: ${ADDITIONAL_INSTALL_TARGET}")
  
### compile option (Debug) 
set(CMAKE_CXX_FLAGS_DEBUG "-W -g")
set(CMAKE_C_FLAGS_DEBUG "-W -g")

### compile option (Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_C_FLAGS_RELEASE "-O2")

### include SWIG
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

### Ruby setting
find_package(Ruby 2.0 REQUIRED)
set(RUBY_LINK_FLAGS " ")
find_path(RUBY_CONFIG_FILE_DIR
  NAMES ruby/config.h
  PATHS ${RUBY_HDR_DIR}/*
  NO_DEFAULT_PATH)
message("-- RUBY_HDR_DIR: ${RUBY_HDR_DIR}")
message("-- RUBY_CONFIG_FILE_DIR: ${RUBY_CONFIG_FILE_DIR}")
set(RUBY_INCLUDE_DIRS ${RUBY_HDR_DIR} ${RUBY_CONFIG_FILE_DIR})
set(RUBY_LIBRARY "")
if(APPLE)
  set(RUBY_LINK_FLAGS "-Wl,-undefined,dynamic_lookup -Wl,-multiply_defined,suppress -Wl,-flat_namespace")
endif()
message("-- Ruby include: ${RUBY_INCLUDE_DIRS}")
message("-- Ruby library: ${RUBY_LIBRARY}")

### SWIG setting
include_directories(
  ${RUBY_EXT_INCLUDE_DIRS}
  ${RUBY_INCLUDE_DIRS}
  )

link_directories(
  ${RUBY_EXT_LIBRARY_DIRS}
  )

set(CMAKE_SWIG_FLAGS "")

set_source_files_properties(${SWIG_IF_FILE} PROPERTIES CPLUSPLUS ON)
if(SWIG_FLAGS)
  foreach(s ${SWIG_FLAGS})
    set(sflags ${sflags} ${s})
  endforeach()
  set_source_files_properties(${SWIG_IF_FILE} PROPERTIES SWIG_FLAGS "${sflags}")
endif()

swig_add_library(${TARGET_EXT_LIBRARY}
  LANGUAGE ruby
  SOURCES ${SWIG_IF_FILE})
swig_link_libraries(${TARGET_EXT_LIBRARY}
  ${RUBY_LIBRARY}
  ${RUBY_EXT_LIBRARIES})

if(APPLE)
  set(RUBY_BINDING_SUFFIX ".bundle")
else(APPLE)
  set(RUBY_BINDING_SUFFIX ".so")
endif(APPLE)

set_target_properties(${TARGET_EXT_LIBRARY}
  PROPERTIES
  OUTPUT_NAME ${TARGET_EXT_LIBRARY_OUTPUT}
  PREFIX ""
  SUFFIX ${RUBY_BINDING_SUFFIX}
  LINK_FLAGS ${RUBY_LINK_FLAGS}
  )

### install setting
if(NOT EXT_LIB_INSTALL_DESTINATION)
  set(EXT_LIB_INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/ruby)
endif()

install(TARGETS ${TARGET_EXT_LIBRARY}
  LIBRARY
  DESTINATION ${EXT_LIB_INSTALL_DESTINATION}
  )

if(ADDITIONAL_INSTALL_TARGET)
  install(FILES ${ADDITIONAL_INSTALL_TARGET}
    DESTINATION ${EXT_LIB_INSTALL_DESTINATION}
    )
endif()
