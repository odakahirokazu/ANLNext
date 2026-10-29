find_package(Threads REQUIRED)
add_executable(anlnext_loop_control loop_control.cc)
target_link_libraries(anlnext_loop_control PRIVATE ANLNext Threads::Threads)
foreach(mode serial ordered skip quit quit_all exception anl_exception terminate_exception critical)
  add_test(NAME loop_control_${mode} COMMAND anlnext_loop_control ${mode})
  set_tests_properties(loop_control_${mode} PROPERTIES TIMEOUT 15)
endforeach()

add_executable(anlnext_parameter_selection parameter_selection.cc)
target_link_libraries(anlnext_parameter_selection PRIVATE ANLNext)
add_test(NAME parameter_selection COMMAND anlnext_parameter_selection)
set_tests_properties(parameter_selection PROPERTIES TIMEOUT 15)

add_executable(anlnext_parameter_defaults parameter_defaults.cc)
target_link_libraries(anlnext_parameter_defaults PRIVATE ANLNext)
add_test(NAME parameter_defaults COMMAND anlnext_parameter_defaults)
set_tests_properties(parameter_defaults PROPERTIES TIMEOUT 15)

add_executable(anlnext_parameter_rebinding parameter_rebinding.cc)
target_link_libraries(anlnext_parameter_rebinding PRIVATE ANLNext)
add_test(NAME parameter_rebinding COMMAND anlnext_parameter_rebinding)
set_tests_properties(parameter_rebinding PROPERTIES TIMEOUT 15)

add_executable(anlnext_submodule submodule.cc)
target_link_libraries(anlnext_submodule PRIVATE ANLNext)
add_test(NAME submodule COMMAND anlnext_submodule)
set_tests_properties(submodule PROPERTIES TIMEOUT 15)

add_subdirectory(bindings)

add_executable(anlnext_module_requirements module_requirements.cc)
target_link_libraries(anlnext_module_requirements PRIVATE ANLNext)
add_test(NAME module_requirements COMMAND anlnext_module_requirements)
set_tests_properties(module_requirements PROPERTIES TIMEOUT 15)
