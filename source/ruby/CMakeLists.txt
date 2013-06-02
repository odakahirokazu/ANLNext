########################################################
set(TARGET_EXT_LIBRARY anl)
set(SWIG_IF_FILE ${TARGET_EXT_LIBRARY}.i)
set(RUBY_EXT_INCLUDE_DIRS ../include ${BOOST_INC_DIR})
set(RUBY_EXT_LIBRARY_DIRS )
set(RUBY_EXT_LIBRARIES ANLNext)
set(SWIG_FLAGS )
set(ADDITIONAL_INSTALL_TARGET )
########################################################

include(CreateSwigRuby)

if(ANL_INSTALL_HEADERS)
  install(FILES ${SWIG_IF_FILE}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/anl/ruby)
endif(ANL_INSTALL_HEADERS)
