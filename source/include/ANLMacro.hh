#ifndef ANLNEXT_ANLMacro_H
#define ANLNEXT_ANLMacro_H 1

#define DEFINE_ANL_MODULE(CLASS_NAME, VERSION) \
  using ThisModuleClass = CLASS_NAME; \
  std::string __module_name__() const override \
  { return #CLASS_NAME; } \
  std::string __module_version__() const override \
  { return #VERSION; }

#define ENABLE_PARALLEL_RUN() \
  std::unique_ptr<BasicModule> __clone__() override \
  { return make_clone(new ThisModuleClass(*this)); }

#endif /* ANLNEXT_ANLMacro_H */
