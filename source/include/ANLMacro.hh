#ifndef ANLNEXT_ANLMacro_H
#define ANLNEXT_ANLMacro_H 1

#define DEFINE_ANL_MODULE(CLASS_NAME, VERSION)    \
  using mod_class = CLASS_NAME;                   \
  std::string __module_name__() const override    \
  { return #CLASS_NAME; }                         \
  std::string __module_version__() const override \
  { return #VERSION; }

#define ENABLE_PARALLEL_RUN()                                    \
  std::unique_ptr<BasicModule> __clone__() const override        \
  { return std::unique_ptr<mod_class>(new mod_class(*this)); }   \
  std::unique_ptr<BasicModule> __duplicate__() override          \
  { return help_duplication(__clone__()); }                      \
  mod_class* singleton_self()                                    \
  { return static_cast<mod_class*>(__singleton_ptr__()); }       \
  const mod_class* singleton_self() const                        \
  { return static_cast<const mod_class*>(__singleton_ptr__()); }

#endif /* ANLNEXT_ANLMacro_H */
