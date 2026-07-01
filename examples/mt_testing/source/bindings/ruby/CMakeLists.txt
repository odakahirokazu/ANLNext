########################################################
include(ANLNextSwigRuby)
anlnext_add_swig_ruby_binding(myPackageMT
  INTERFACE_FILE myPackageMT.i
  LIBRARIES ANLNext MyPackageMT
)
########################################################
