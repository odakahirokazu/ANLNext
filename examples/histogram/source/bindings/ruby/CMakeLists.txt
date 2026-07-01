########################################################
include(ANLNextSwigRuby)
anlnext_add_swig_ruby_binding(testHistogram
  INTERFACE_FILE testHistogram.i
  LIBRARIES ANLNext TestHistogram
)
########################################################
