########################################################
include(ANLNextSwigRuby)
anlnext_add_swig_ruby_binding(ANL
  MODULE_NAME ANL
  INTERFACE_FILE interface/ANL.i
  LIBRARIES ANLNext
  INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/ruby/anlnext
)
########################################################

if(ANLNEXT_INSTALL_HEADERS)
  install(FILES interface/ANL.i
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/anlnext/ruby
  )
endif(ANLNEXT_INSTALL_HEADERS)

########################################################
# install Ruby libraries written in Ruby
########################################################
install(DIRECTORY library/
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/ruby
  FILES_MATCHING PATTERN "*.rb"
)
