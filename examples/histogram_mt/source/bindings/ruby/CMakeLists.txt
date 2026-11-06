include(ANLNextSwigRuby)
anlnext_add_swig_ruby_binding(testHistogramMT
  INTERFACE_FILE testHistogramMT.i
  LIBRARIES ANLNext TestHistogramMT
)
