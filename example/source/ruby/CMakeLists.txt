find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(Ruby)
include_directories(${RUBY_INCLUDE_PATH})

set(PACKAGE myPackage)
set(IF_FILE ${PACKAGE}.i)

# compile option (Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_DEBUG "-g")

# compile option (Release)
set(CMAKE_CXX_FLAGS_RELEASE "-W -O2")
set(CMAKE_C_FLAGS_RELEASE "-W -O2")

include_directories(
  ../include
  ${ANLNEXT_DIR}/ruby
  )

set(CMAKE_SWIG_FLAGS "")

link_directories(
  ${AN_LIB_INSTALL_PATH}
  )

set(sflags "")
add_definitions(${sflags})
set(sflags ${sflags} -includeall)
set_source_files_properties(${IF_FILE} PROPERTIES CPLUSPLUS ON)
set_source_files_properties(${IF_FILE} PROPERTIES SWIG_FLAGS "${sflags}")

swig_add_module(${PACKAGE} ruby ${IF_FILE})
swig_link_libraries(${PACKAGE}
  ${RUBY_LIBRARY}
  ${MY_LIBRARY}
  ANLNext
  )

set_target_properties(${PACKAGE}
  PROPERTIES
  PREFIX ""
  SUFFIX ".bundle"
  )

install(TARGETS ${PACKAGE} LIBRARY DESTINATION ${AN_LIB_INSTALL_PATH}/ruby)
