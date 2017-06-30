#ifndef ANLNEXT_ANLMacro_H
#define ANLNEXT_ANLMacro_H 1

#define ANL_USE_OVERRIDE_SPECIFIER 1
#if ANL_USE_OVERRIDE_SPECIFIER
#define DEFINE_ANL_MODULE(CLASS_NAME, VERSION) \
  std::string __module_name__() const override \
  { return #CLASS_NAME; } \
  std::string __module_version__() const override \
  { return #VERSION; }
#else
#define DEFINE_ANL_MODULE(CLASS_NAME, VERSION) \
  std::string __module_name__() const \
  { return #CLASS_NAME; } \
  std::string __module_version__() const \
  { return #VERSION; }
#endif

#endif /* ANLNEXT_ANLMacro_H */
