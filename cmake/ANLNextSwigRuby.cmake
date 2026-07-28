# ANLNextSwigRuby.cmake

include_guard()

find_package(SWIG REQUIRED)
include(UseSWIG)

if(POLICY CMP0078)
  cmake_policy(SET CMP0078 NEW)
endif()

if(POLICY CMP0086)
  cmake_policy(SET CMP0086 NEW)
endif()

function(_anlnext_check_required name value)
  if(NOT value)
    message(FATAL_ERROR "${name} is required")
  endif()
endfunction()

function(anlnext_add_swig_ruby_binding target)
  cmake_parse_arguments(ARG
    ""
    "MODULE_NAME;INTERFACE_FILE;INSTALL_DESTINATION"
    "LIBRARIES;INCLUDE_DIRECTORIES;SWIG_FLAGS;COMPILE_DEFINITIONS;INSTALL_FILES"
    ${ARGN}
  )

  _anlnext_check_required("INTERFACE_FILE" "${ARG_INTERFACE_FILE}")

  if(NOT ARG_MODULE_NAME)
    set(ARG_MODULE_NAME ${target})
  endif()

  if(NOT ARG_INSTALL_DESTINATION)
    set(ARG_INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/ruby")
  endif()

  find_package(Ruby 3.0 REQUIRED COMPONENTS Interpreter Development)

  set_property(SOURCE ${ARG_INTERFACE_FILE} PROPERTY CPLUSPLUS ON)
  set_property(SOURCE ${ARG_INTERFACE_FILE} PROPERTY USE_TARGET_INCLUDE_DIRECTORIES TRUE)
  set_property(SOURCE ${ARG_INTERFACE_FILE} PROPERTY SWIG_MODULE_NAME ${ARG_MODULE_NAME})

  if(ARG_SWIG_FLAGS)
    set_property(SOURCE ${ARG_INTERFACE_FILE} PROPERTY SWIG_FLAGS ${ARG_SWIG_FLAGS})
  endif()

  swig_add_library(${target}
    TYPE MODULE
    LANGUAGE ruby
    SOURCES ${ARG_INTERFACE_FILE}
  )

  set_target_properties(${target}
    PROPERTIES
    LINKER_LANGUAGE CXX
    OUTPUT_NAME ${ARG_MODULE_NAME}
    PREFIX ""
  )

  target_include_directories(${target}
    PRIVATE
    ${ARG_INCLUDE_DIRECTORIES}
    ${Ruby_INCLUDE_DIRS}
  )

  target_include_directories(${target}
    SYSTEM PRIVATE
    ${Ruby_INCLUDE_DIRS}
  )

  target_compile_definitions(${target}
    PRIVATE
    ${ARG_COMPILE_DEFINITIONS}
  )

  target_compile_options(${target}
    PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:
      -Wno-unused-parameter
      -Wno-deprecated-declarations
      -Wno-ignored-qualifiers
      -Wno-deprecated-copy-with-user-provided-copy
    >
  )

  target_link_libraries(${target}
    PRIVATE
    ${ARG_LIBRARIES}
    ${Ruby_LIBRARIES}
  )

  if(APPLE)
    target_link_options(${target}
      PRIVATE
      -Wl,-undefined,dynamic_lookup
    )

    set_target_properties(${target}
      PROPERTIES
      SUFFIX ".bundle"
    )
  else()
    set_target_properties(${target}
      PROPERTIES
      SUFFIX ".so"
    )
  endif()

  install(TARGETS ${target}
    LIBRARY DESTINATION ${ARG_INSTALL_DESTINATION}
  )

  if(ARG_INSTALL_FILES)
    install(FILES ${ARG_INSTALL_FILES}
      DESTINATION ${ARG_INSTALL_DESTINATION}
    )
  endif()
endfunction()
