########################################################
set(TARGET_EXT_LIBRARY ANL)
set(EXT_LIB_INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/ruby/anlnext)
set(SWIG_IF_FILE interface/${TARGET_EXT_LIBRARY}.i)
set(RUBY_EXT_INCLUDE_DIRS ../include ${BOOST_INC_DIR})
set(RUBY_EXT_LIBRARY_DIRS )
set(RUBY_EXT_LIBRARIES ANLNext)
set(SWIG_FLAGS )
set(ADDITIONAL_INSTALL_TARGET )
########################################################

include(CreateSwigRuby)

if(ANLNEXT_INSTALL_HEADERS)
  install(FILES ${SWIG_IF_FILE}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/anlnext/ruby)
endif(ANLNEXT_INSTALL_HEADERS)

########################################################
# install Ruby libraries written in Ruby
########################################################
install(DIRECTORY library/
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/ruby
  FILES_MATCHING PATTERN "*.rb")
