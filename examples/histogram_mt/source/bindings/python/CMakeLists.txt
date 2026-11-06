include(ANLNextSwigPython)
anlnext_add_swig_python_binding(testHistogramMTPy
  INTERFACE_FILE testHistogramMTPy.i
  LIBRARIES ANLNext TestHistogramMT
)
