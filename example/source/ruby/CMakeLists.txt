########################################################
set(TARGET_EXT_LIBRARY myPackage)
set(SWIG_IF_FILE ${TARGET_EXT_LIBRARY}.i)
########################################################

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(Ruby)
include_directories(${RUBY_INCLUDE_PATH})

### compile option (Debug) 
set(CMAKE_CXX_FLAGS_DEBUG "-W -g")
set(CMAKE_C_FLAGS_DEBUG "-W -g")

### compile option (Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_C_FLAGS_RELEASE "-O2")

set(CMAKE_SWIG_FLAGS "")

include_directories(
  ../include
  ${ANLNEXT_INC_DIR}/ruby
  )

link_directories(
  ${ANLNEXT_LIB_PATH}
  )

set_source_files_properties(${SWIG_IF_FILE} PROPERTIES CPLUSPLUS ON)
# set_source_files_properties(${SWIG_IF_FILE} PROPERTIES SWIG_FLAGS "-includeall")
swig_add_module(${TARGET_EXT_LIBRARY} ruby ${SWIG_IF_FILE})
swig_link_libraries(${TARGET_EXT_LIBRARY}
  ${MY_LIBRARY} ${RUBY_LIBRARY} ANLNext)

if(APPLE)
  set(RUBY_BINDING_SUFFIX ".bundle")
else(APPLE)
  set(RUBY_BINDING_SUFFIX ".so")
endif(APPLE)

set_target_properties(${TARGET_EXT_LIBRARY}
  PROPERTIES
  PREFIX ""
  SUFFIX ${RUBY_BINDING_SUFFIX})

install(TARGETS ${TARGET_EXT_LIBRARY}
  LIBRARY
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/ruby)

# if(ANL_INSTALL_HEADERS)
#  install(FILES ${SWIG_IF_FILE}
#    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/anl/ruby)
# endif(ANL_INSTALL_HEADERS)
