include(ANLNextSwigPython)
anlnext_add_swig_python_binding(testHistogramPy
  INTERFACE_FILE testHistogramPy.i
  LIBRARIES ANLNext TestHistogram
)
