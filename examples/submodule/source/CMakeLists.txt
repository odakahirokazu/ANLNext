cmake_minimum_required(VERSION 3.30)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(SubmoduleExample LANGUAGES CXX)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  if(DEFINED ENV{ANLNEXT_INSTALL})
    set(ANLNEXT_INSTALL "$ENV{ANLNEXT_INSTALL}" CACHE PATH "ANLNext installation prefix")
  else()
    set(ANLNEXT_INSTALL "$ENV{HOME}" CACHE PATH "ANLNext installation prefix")
  endif()
  find_package(Boost CONFIG 1.80.0 REQUIRED)
  find_path(ANLNEXT_INCLUDE_DIR BasicModule.hh HINTS "${ANLNEXT_INSTALL}/include/anlnext" REQUIRED)
  find_library(ANLNEXT_LIBRARY ANLNext HINTS "${ANLNEXT_INSTALL}/lib" REQUIRED)
  find_path(ANLNEXT_RUBY_INTERFACE_DIR ANL.i HINTS "${ANLNEXT_INCLUDE_DIR}/ruby" REQUIRED)
  find_path(ANLNEXT_PYTHON_INTERFACE_DIR anlnextpy.i HINTS "${ANLNEXT_INCLUDE_DIR}/python" REQUIRED)
  list(APPEND CMAKE_MODULE_PATH "${ANLNEXT_INSTALL}/lib/anlnext")
else()
  set(ANLNEXT_INCLUDE_DIR ${ANLNext_SOURCE_DIR}/source/include)
  set(ANLNEXT_LIBRARY ANLNext)
  set(ANLNEXT_RUBY_INTERFACE_DIR ${ANLNext_SOURCE_DIR}/source/bindings/ruby/interface)
  set(ANLNEXT_PYTHON_INTERFACE_DIR ${ANLNext_SOURCE_DIR}/source/bindings/python/interface)
endif()

add_library(SubmoduleAlgorithms SHARED src/AlgorithmModule.cc src/CalculationAlgorithm.cc)
target_include_directories(SubmoduleAlgorithms PUBLIC include ${ANLNEXT_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
target_link_libraries(SubmoduleAlgorithms PUBLIC ${ANLNEXT_LIBRARY})

include(ANLNextSwigRuby)
anlnext_add_swig_ruby_binding(submoduleExample
  INTERFACE_FILE bindings/ruby/submoduleExample.i
  INCLUDE_DIRECTORIES ${ANLNEXT_RUBY_INTERFACE_DIR}
  LIBRARIES SubmoduleAlgorithms)

include(ANLNextSwigPython)
anlnext_add_swig_python_binding(submoduleExamplePy
  INTERFACE_FILE bindings/python/submoduleExamplePy.i
  INCLUDE_DIRECTORIES ${ANLNEXT_PYTHON_INTERFACE_DIR}
  LIBRARIES SubmoduleAlgorithms)

install(TARGETS SubmoduleAlgorithms LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
