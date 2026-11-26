########################################################
include(ANLNextSwigRuby)
anlnext_add_swig_ruby_binding(ANL
  MODULE_NAME ANL
  INTERFACE_FILE interface/ANL.i
  LIBRARIES ANLNext
  INSTALL_DESTINATION lib/ruby/anlnext
)
########################################################

if(ANLNEXT_INSTALL_HEADERS)
  install(FILES interface/ANL.i
    DESTINATION include/anlnext/ruby
  )
endif(ANLNEXT_INSTALL_HEADERS)

########################################################
# install Ruby libraries written in Ruby
########################################################
install(DIRECTORY library/
  DESTINATION lib/ruby
  FILES_MATCHING PATTERN "*.rb"
)
