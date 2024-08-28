########################################################
set(TARGET_EXT_LIBRARY anlpy)
set(EXT_LIB_INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/python/anlnext)
set(SWIG_IF_FILE interface/${TARGET_EXT_LIBRARY}.i)
set(PYTHON_EXT_INCLUDE_DIRS ../include ${BOOST_INC_DIR})
set(PYTHON_EXT_LIBRARY_DIRS )
set(PYTHON_EXT_LIBRARIES ANLNext)
set(SWIG_FLAGS )
set(ADDITIONAL_INSTALL_TARGET )
########################################################

include(CreateSwigPython)

if(ANLNEXT_INSTALL_HEADERS)
  install(FILES ${SWIG_IF_FILE}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/anlnext/python)
endif(ANLNEXT_INSTALL_HEADERS)

########################################################
# install Python libraries written in Python
########################################################
install(DIRECTORY library/
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/python
  FILES_MATCHING PATTERN "*.py")
