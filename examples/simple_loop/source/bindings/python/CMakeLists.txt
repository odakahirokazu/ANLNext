########################################################
include(ANLNextSwigPython)
anlnext_add_swig_python_binding(myPackagePy
  INTERFACE_FILE myPackagePy.i
  INCLUDE_DIRECTORIESS ${ANLNEXT_INC_DIR}
  LIBRARIES ANLNext MyPackage
)
########################################################
