########################################################
include(ANLNextSwigPython)
anlnext_add_swig_python_binding(myPackageMTPy
  INTERFACE_FILE myPackageMTPy.i
  INCLUDE_DIRECTORIESS ${ANLNEXT_INC_DIR}
  LIBRARIES ANLNext MyPackageMT
)
########################################################
