install(FILES CreateSwigRuby.cmake CreateSwigPython.cmake
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/anlnext)

if(ANL_USE_EXE)
  if(ANL_INSTALL_CMAKE_FILES)
    install(FILES gen_anl.cmake
      DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/anlnext)
  endif(ANL_INSTALL_CMAKE_FILES)
endif(ANL_USE_EXE)
