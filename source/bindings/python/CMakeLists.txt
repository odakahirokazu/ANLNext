########################################################
include(ANLNextSwigPython)
anlnext_add_swig_python_binding(anlnextpy
  INTERFACE_FILE interface/anlnextpy.i
  LIBRARIES ANLNext
  INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/python/anlnext
)
########################################################

if(ANLNEXT_INSTALL_HEADERS)
  install(FILES interface/anlnextpy.i
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/anlnext/python
  )
endif(ANLNEXT_INSTALL_HEADERS)

########################################################
# install Python libraries written in Python
########################################################
install(DIRECTORY library/
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/python
  FILES_MATCHING PATTERN "*.py"
)
