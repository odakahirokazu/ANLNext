find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(Ruby)
include_directories(${RUBY_INCLUDE_PATH})
# include_directories(${RUBY_HDR_DIR})
# message("Ruby header directory: ${RUBY_HDR_DIR}")

# workaround Ruby 1.9.2
# add_definitions(-DHAVE_STRUCT_TIMESPEC -DHAVE_STRUCT_TIMEZONE)

# compile option (Debug) 
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_DEBUG "-g")

# compile option (Release)
set(CMAKE_CXX_FLAGS_RELEASE "-W -O2")
set(CMAKE_C_FLAGS_RELEASE "-W -O2")

set(CMAKE_SWIG_FLAGS "")

#include_directories(/usr/local/include /usr/include)
link_directories(${AN_TOP_BUILD}/lib)

set_source_files_properties(anl.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(anl.i PROPERTIES SWIG_FLAGS "-includeall")
swig_add_module(anl ruby anl.i)
swig_link_libraries(anl ${RUBY_LIBRARY} ANLNext)

set_target_properties(anl
 PROPERTIES
  PREFIX ""
  SUFFIX ".bundle"
  LIBRARY_OUTPUT_DIRECTORY ${AN_TOP_BUILD}/lib/ruby
)

install(TARGETS anl LIBRARY DESTINATION ${AN_LIB_INSTALL_PATH}/ruby)
