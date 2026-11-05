find_package(SWIG REQUIRED)
include(UseSWIG)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(Ruby 3.0 REQUIRED COMPONENTS Interpreter Development)

add_library(submodule_example STATIC SubmoduleExample.cc MapExample.cc UnsignedIntegerExample.cc RequirementExample.cc DefinitionStatusExample.cc)
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
target_compile_options(submodule_python PRIVATE
  $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:-Wno-unused-parameter>)
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

add_test(NAME submodule_parameters_python COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/submodule_parameters.py)
set_tests_properties(submodule_parameters_python PROPERTIES TIMEOUT 15
  ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python:${CMAKE_CURRENT_BINARY_DIR}")
add_test(NAME submodule_parameters_ruby COMMAND ${Ruby_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/submodule_parameters.rb)
set_tests_properties(submodule_parameters_ruby PROPERTIES TIMEOUT 15
  ENVIRONMENT "RUBYLIB=${CMAKE_CURRENT_BINARY_DIR}/ruby:${CMAKE_CURRENT_BINARY_DIR}")

add_test(NAME yaml_parameters_python COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/yaml_parameters.py)
set_tests_properties(yaml_parameters_python PROPERTIES TIMEOUT 15
  ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python:${CMAKE_CURRENT_BINARY_DIR}")
add_test(NAME yaml_parameters_ruby COMMAND ${Ruby_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/yaml_parameters.rb)
set_tests_properties(yaml_parameters_ruby PROPERTIES TIMEOUT 15
  ENVIRONMENT "RUBYLIB=${CMAKE_CURRENT_BINARY_DIR}/ruby:${CMAKE_CURRENT_BINARY_DIR}")

add_subdirectory(${PROJECT_SOURCE_DIR}/examples/submodule/source submodule_example)
foreach(parallels 1 3)
  add_test(NAME submodule_example_python_${parallels}
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/examples/submodule/run_py/run_submodule.py ${parallels})
  set_tests_properties(submodule_example_python_${parallels} PROPERTIES TIMEOUT 15
    ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python:${CMAKE_CURRENT_BINARY_DIR}/submodule_example")
  add_test(NAME submodule_example_ruby_${parallels}
    COMMAND ${Ruby_EXECUTABLE} ${PROJECT_SOURCE_DIR}/examples/submodule/run/run_submodule.rb ${parallels})
  set_tests_properties(submodule_example_ruby_${parallels} PROPERTIES TIMEOUT 15
    ENVIRONMENT "RUBYLIB=${CMAKE_CURRENT_BINARY_DIR}/ruby:${CMAKE_CURRENT_BINARY_DIR}/submodule_example")
  add_test(NAME submodule_example_yaml_python_${parallels}
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/examples/submodule/run_py/run_submodule.py ${parallels}
      ${PROJECT_SOURCE_DIR}/examples/submodule/parameters.yaml)
  set_tests_properties(submodule_example_yaml_python_${parallels} PROPERTIES TIMEOUT 15
    ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python:${CMAKE_CURRENT_BINARY_DIR}/submodule_example")
  add_test(NAME submodule_example_yaml_ruby_${parallels}
    COMMAND ${Ruby_EXECUTABLE} ${PROJECT_SOURCE_DIR}/examples/submodule/run/run_submodule.rb ${parallels}
      ${PROJECT_SOURCE_DIR}/examples/submodule/parameters.yaml)
  set_tests_properties(submodule_example_yaml_ruby_${parallels} PROPERTIES TIMEOUT 15
    ENVIRONMENT "RUBYLIB=${CMAKE_CURRENT_BINARY_DIR}/ruby:${CMAKE_CURRENT_BINARY_DIR}/submodule_example")
endforeach()

add_executable(anlnext_map_parameter ../map_parameter.cc)
target_link_libraries(anlnext_map_parameter PRIVATE submodule_example)
add_test(NAME map_parameter COMMAND anlnext_map_parameter)
set_tests_properties(map_parameter PROPERTIES TIMEOUT 15)

add_test(NAME map_parameter_python COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/map_parameter.py)
set_tests_properties(map_parameter_python PROPERTIES TIMEOUT 15
  ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python:${CMAKE_CURRENT_BINARY_DIR}")

add_test(NAME vector_parameter_python COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/vector_parameter.py)
set_tests_properties(vector_parameter_python PROPERTIES TIMEOUT 15
  ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python:${CMAKE_CURRENT_BINARY_DIR}")

add_test(NAME definition_status_python COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/definition_status.py)
set_tests_properties(definition_status_python PROPERTIES TIMEOUT 15
  ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python:${CMAKE_CURRENT_BINARY_DIR}")

add_test(NAME multiple_runs_python COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/multiple_runs.py)
set_tests_properties(multiple_runs_python PROPERTIES TIMEOUT 15
  ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python:${CMAKE_CURRENT_BINARY_DIR}")
add_test(NAME multiple_runs_ruby COMMAND ${Ruby_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/multiple_runs.rb)
set_tests_properties(multiple_runs_ruby PROPERTIES TIMEOUT 15
  ENVIRONMENT "RUBYLIB=${CMAKE_CURRENT_BINARY_DIR}/ruby:${CMAKE_CURRENT_BINARY_DIR}")

add_test(NAME parameter_output_python COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/parameter_output.py)
set_tests_properties(parameter_output_python PROPERTIES TIMEOUT 15
  ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python:${CMAKE_CURRENT_BINARY_DIR}")
add_test(NAME parameter_output_ruby COMMAND ${Ruby_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/parameter_output.rb)
set_tests_properties(parameter_output_ruby PROPERTIES TIMEOUT 15
  ENVIRONMENT "RUBYLIB=${CMAKE_CURRENT_BINARY_DIR}/ruby:${CMAKE_CURRENT_BINARY_DIR}")
add_test(NAME map_parameter_ruby COMMAND ${Ruby_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/map_parameter.rb)
set_tests_properties(map_parameter_ruby PROPERTIES TIMEOUT 15
  ENVIRONMENT "RUBYLIB=${CMAKE_CURRENT_BINARY_DIR}/ruby:${CMAKE_CURRENT_BINARY_DIR}")

add_test(NAME script_generation_ruby COMMAND ${Ruby_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/script_generation.rb)
set_tests_properties(script_generation_ruby PROPERTIES TIMEOUT 15
  ENVIRONMENT "RUBYLIB=${CMAKE_CURRENT_BINARY_DIR}/ruby:${CMAKE_CURRENT_BINARY_DIR}")

add_test(NAME record_boolean_python COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/record_boolean.py)
set_tests_properties(record_boolean_python PROPERTIES TIMEOUT 15
  ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python:${CMAKE_CURRENT_BINARY_DIR}")
add_test(NAME record_boolean_ruby COMMAND ${Ruby_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/record_boolean.rb)
set_tests_properties(record_boolean_ruby PROPERTIES TIMEOUT 15
  ENVIRONMENT "RUBYLIB=${CMAKE_CURRENT_BINARY_DIR}/ruby:${CMAKE_CURRENT_BINARY_DIR}")

add_executable(anlnext_unsigned_integer ../unsigned_integer.cc)
target_link_libraries(anlnext_unsigned_integer PRIVATE submodule_example)
add_test(NAME unsigned_integer COMMAND anlnext_unsigned_integer)
set_tests_properties(unsigned_integer PROPERTIES TIMEOUT 15)

add_test(NAME unsigned_integer_python COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/unsigned_integer.py)
set_tests_properties(unsigned_integer_python PROPERTIES TIMEOUT 15
  ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python:${CMAKE_CURRENT_BINARY_DIR}")
add_test(NAME unsigned_integer_ruby COMMAND ${Ruby_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/unsigned_integer.rb)
set_tests_properties(unsigned_integer_ruby PROPERTIES TIMEOUT 15
  ENVIRONMENT "RUBYLIB=${CMAKE_CURRENT_BINARY_DIR}/ruby:${CMAKE_CURRENT_BINARY_DIR}")

add_test(NAME module_requirements_python COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/module_requirements.py)
set_tests_properties(module_requirements_python PROPERTIES TIMEOUT 15
  ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python:${CMAKE_CURRENT_BINARY_DIR}")
add_test(NAME module_requirements_ruby COMMAND ${Ruby_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/module_requirements.rb)
set_tests_properties(module_requirements_ruby PROPERTIES TIMEOUT 15
  ENVIRONMENT "RUBYLIB=${CMAKE_CURRENT_BINARY_DIR}/ruby:${CMAKE_CURRENT_BINARY_DIR}")
