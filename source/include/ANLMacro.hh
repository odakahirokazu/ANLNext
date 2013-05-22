#ifndef ANLNEXT_ANLMacro_H
#define ANLNEXT_ANLMacro_H 1

#define DEFINE_ANL_MODULE(CLASS_NAME, VERSION)      \
  virtual std::string __module_name__() const       \
  { return #CLASS_NAME; }                           \
  virtual std::string __module_version__() const    \
  { return #VERSION; }


#endif /* ANLNEXT_ANLMacro_H */
