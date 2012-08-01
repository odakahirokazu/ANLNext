set(MakeHeader ${ANLNEXT_DIR}/script/makeANLNextHeader.rb)
set(MakeMain ${ANLNEXT_DIR}/script/makeANLNextMain.rb)

foreach(app ${app_list})
  add_custom_command(
    OUTPUT ${app}.hh
    COMMAND ${MakeHeader}
    ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${ANL_DEF_DIR}/${app}.def
    ${CMAKE_CURRENT_BINARY_DIR}/${app}.hh
    DEPENDS ./${ANL_DEF_DIR}/${app}.def
    )
  add_custom_command(
    OUTPUT ${app}.cc
    COMMAND ${MakeMain}
    ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${ANL_DEF_DIR}/${app}.def
    ${USEROOT}
    ${CMAKE_CURRENT_BINARY_DIR}/${app}.cc
    DEPENDS ./${ANL_DEF_DIR}/${app}.def ${app}.hh
    )
  
  add_executable(${app} ${app}.cc ${modules})
  target_link_libraries(${app} ${ANL_EXE_LIB} ANLNext)
  
  set_property(TARGET ${app} PROPERTY
    RUNTIME_OUTPUT_DIRECTORY ${ANL_BIN_DIR})
  install(TARGETS ${app} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
endforeach(app)
