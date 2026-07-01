########################################################
include(ANLNextSwigRuby)
anlnext_add_swig_ruby_binding(myPackage
  INTERFACE_FILE myPackage.i
  LIBRARIES ANLNext MyPackage
)
########################################################
