install(
  FILES
  ANLNextSwigRuby.cmake ANLNextSwigPython.cmake
  DESTINATION
  lib/anlnext
)

if(ANL_USE_EXE)
  if(ANL_INSTALL_CMAKE_FILES)
    install(FILES gen_anl.cmake
      DESTINATION lib/anlnext)
  endif(ANL_INSTALL_CMAKE_FILES)
endif(ANL_USE_EXE)
