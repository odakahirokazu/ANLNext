find_package(SWIG REQUIRED)
include(UseSWIG)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(Ruby 3.0 REQUIRED COMPONENTS Interpreter Development)

add_library(submodule_example STATIC SubmoduleExample.cc ScalarMapExample.cc)
set_target_properties(submodule_example PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(submodule_example PUBLIC ANLNext)
target_include_directories(submodule_example PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_property(SOURCE submodule_python.i PROPERTY SWIG_FLAGS -builtin)

foreach(language python ruby)
  set(interface submodule_${language}.i)
  set_source_files_properties(${interface} PROPERTIES
    CPLUSPLUS ON USE_TARGET_INCLUDE_DIRECTORIES TRUE)
  swig_add_library(submodule_${language} TYPE MODULE LANGUAGE ${language} SOURCES ${interface})
  target_include_directories(submodule_${language} PRIVATE
    ${PROJECT_SOURCE_DIR}/source/bindings/${language}/interface)
  target_link_libraries(submodule_${language} PRIVATE submodule_example)
endforeach()
target_include_directories(submodule_python SYSTEM PRIVATE ${Python3_INCLUDE_DIRS})
target_link_libraries(submodule_python PRIVATE Python3::Python)
target_include_directories(submodule_ruby SYSTEM PRIVATE ${Ruby_INCLUDE_DIRS})
target_link_libraries(submodule_ruby PRIVATE ${Ruby_LIBRARIES})
set_target_properties(submodule_ruby PROPERTIES PREFIX "")
if(APPLE)
  set_target_properties(submodule_ruby PROPERTIES SUFFIX ".bundle")
endif()
target_compile_options(submodule_ruby PRIVATE
  $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:-Wno-unused-parameter;-Wno-deprecated-declarations;-Wno-missing-field-initializers>)

# Load the build's bindings without installing over the user's ANLNext.
file(COPY ${PROJECT_SOURCE_DIR}/source/bindings/python/library/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/python)
file(COPY ${PROJECT_SOURCE_DIR}/source/bindings/ruby/library/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ruby)
file(CREATE_LINK ${PROJECT_BINARY_DIR}/source/bindings/python/anlnextpy.py
  ${CMAKE_CURRENT_BINARY_DIR}/python/anlnext/anlnextpy.py SYMBOLIC)
add_custom_target(submodule_binding_runtime ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:anlnextpy> ${CMAKE_CURRENT_BINARY_DIR}/python/anlnext/
  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:ANL> ${CMAKE_CURRENT_BINARY_DIR}/ruby/anlnext/
  DEPENDS anlnextpy ANL)

add_test(NAME submodule_python COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/submodule.py)
set_tests_properties(submodule_python PROPERTIES TIMEOUT 15
  ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python:${CMAKE_CURRENT_BINARY_DIR}")
add_test(NAME submodule_ruby COMMAND ${Ruby_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/submodule.rb)
set_tests_properties(submodule_ruby PROPERTIES TIMEOUT 15
  ENVIRONMENT "RUBYLIB=${CMAKE_CURRENT_BINARY_DIR}/ruby:${CMAKE_CURRENT_BINARY_DIR}")

add_executable(anlnext_scalar_map ../scalar_map.cc)
target_link_libraries(anlnext_scalar_map PRIVATE submodule_example)
add_test(NAME scalar_map COMMAND anlnext_scalar_map)
set_tests_properties(scalar_map PROPERTIES TIMEOUT 15)

add_test(NAME scalar_map_python COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scalar_map.py)
set_tests_properties(scalar_map_python PROPERTIES TIMEOUT 15
  ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python:${CMAKE_CURRENT_BINARY_DIR}")
add_test(NAME scalar_map_ruby COMMAND ${Ruby_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scalar_map.rb)
set_tests_properties(scalar_map_ruby PROPERTIES TIMEOUT 15
  ENVIRONMENT "RUBYLIB=${CMAKE_CURRENT_BINARY_DIR}/ruby:${CMAKE_CURRENT_BINARY_DIR}")
