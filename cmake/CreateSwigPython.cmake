### CreateSwigPython
### 2024-08-28 | Hirokazu Odaka
###

### policy

### pre-process
if (NOT DEFINED TARGET_EXT_LIBRARY_OUTPUT)
  set(TARGET_EXT_LIBRARY_OUTPUT ${TARGET_EXT_LIBRARY})
endif()

### message
message("-- CreateSwigPython: ${TARGET_EXT_LIBRARY}")
message("-- Output name: ${TARGET_EXT_LIBRARY_OUTPUT}")
message("-- SWIG interface file: ${SWIG_IF_FILE}")
message("-- INCLUDE_DIRS: ${PYTHON_EXT_INCLUDE_DIRS}")
message("-- LIBRARY_DIRS: ${PYTHON_EXT_LIBRARY_DIRS}")
message("-- LIBRARIES: ${PYTHON_EXT_LIBRARIES}")
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

### Python setting
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

### SWIG setting
include_directories(
  ${PYTHON_EXT_INCLUDE_DIRS}
  ${Python3_INCLUDE_DIRS}
  )

link_directories(
  ${PYTHON_EXT_LIBRARY_DIRS}
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
  LANGUAGE python
  SOURCES ${SWIG_IF_FILE})
swig_link_libraries(${TARGET_EXT_LIBRARY}
  ${Python3_LIBRARIES}
  ${PYTHON_EXT_LIBRARIES})

### install setting
if(NOT EXT_LIB_INSTALL_DESTINATION)
  set(EXT_LIB_INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/python)
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
