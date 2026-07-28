# ANLNextSwigPython.cmake

include_guard()

find_package(SWIG REQUIRED)
include(UseSWIG)

if(POLICY CMP0078)
  cmake_policy(SET CMP0078 NEW)
endif()

if(POLICY CMP0086)
  cmake_policy(SET CMP0086 NEW)
endif()

function(anlnext_add_swig_python_binding target)
  cmake_parse_arguments(ARG
    ""
    "INTERFACE_FILE;INSTALL_DESTINATION"
    "LIBRARIES;INCLUDE_DIRECTORIES;COMPILE_DEFINITIONS;SWIG_FLAGS;INSTALL_FILES"
    ${ARGN}
  )

  if(NOT ARG_INTERFACE_FILE)
    message(FATAL_ERROR "INTERFACE_FILE is required")
  endif()

  if(NOT ARG_INSTALL_DESTINATION)
    set(ARG_INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/python")
  endif()

  find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

  set(_swig_flags "-builtin")
  if(ARG_SWIG_FLAGS)
    list(APPEND _swig_flags ${ARG_SWIG_FLAGS})
  endif()

  set_property(SOURCE ${ARG_INTERFACE_FILE} PROPERTY CPLUSPLUS ON)
  set_property(SOURCE ${ARG_INTERFACE_FILE} PROPERTY USE_TARGET_INCLUDE_DIRECTORIES TRUE)
  set_property(SOURCE ${ARG_INTERFACE_FILE} PROPERTY SWIG_FLAGS ${_swig_flags})

  swig_add_library(${target}
    LANGUAGE python
    SOURCES ${ARG_INTERFACE_FILE}
  )

  set_target_properties(${target}
    PROPERTIES
    LINKER_LANGUAGE CXX
  )

  target_include_directories(${target}
    PRIVATE
    ${ARG_INCLUDE_DIRECTORIES}
  )

  target_include_directories(${target}
    SYSTEM PRIVATE
    ${Python3_INCLUDE_DIRS}
  )

  target_compile_definitions(${target}
    PRIVATE
    ${ARG_COMPILE_DEFINITIONS}
  )

  target_compile_options(${target}
    PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:
      -Wno-unused-parameter
    >
  )

  target_link_libraries(${target}
    PRIVATE
    ${Python3_LIBRARIES}
    ${ARG_LIBRARIES}
  )

  install(TARGETS ${target}
    LIBRARY DESTINATION ${ARG_INSTALL_DESTINATION}
  )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target}.py
    DESTINATION ${ARG_INSTALL_DESTINATION}
  )

  if(ARG_INSTALL_FILES)
    install(FILES ${ARG_INSTALL_FILES}
      DESTINATION ${ARG_INSTALL_DESTINATION}
    )
  endif()
endfunction()
