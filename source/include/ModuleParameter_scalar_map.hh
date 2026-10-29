#ifndef ANLNEXT_ModuleParameter_scalar_map_H
#define ANLNEXT_ModuleParameter_scalar_map_H 1

namespace anlnext
{

template <typename Key, typename Value>
class ModuleParameterScalarMap : public VModuleParameter
{
  using map_type = std::map<Key, Value>;
public:
  ModuleParameterScalarMap(const std::string& name, map_type* ptr);
  std::string type_name() const override;
  bool ask() override;
  void set_value(const map_type& value) override;
  map_type get_value(const map_type&) const override;
  using VModuleParameter::set_value;
  using VModuleParameter::get_value;
  void add_value_element(ModuleParam_sptr param) override;
  void clear_array() override;
  void clear_container() override;
  std::size_t size_of_container() const override;
  void output(std::ostream& os) const override;
  void input(std::istream& is) override;
  void __get__(void* ptr) const override;
  void __set__(const void* ptr) override;
  boost::property_tree::ptree to_property_tree() const override;
protected:
  ModuleParameterScalarMap(const ModuleParameterScalarMap&) = default;
  virtual map_type& __ref__();
  virtual const map_type& __ref__() const;
private:
  map_type* ptr_;
};

template <>
class ModuleParameter<std::map<int, int>> : public ModuleParameterScalarMap<int, int>
{
public:
  using ModuleParameterScalarMap::ModuleParameterScalarMap;
  std::shared_ptr<VModuleParameter> clone() override;
protected:
  ModuleParameter(const ModuleParameter&) = default;
};

template <>
class ModuleParameter<std::map<int, double>> : public ModuleParameterScalarMap<int, double>
{
public:
  using ModuleParameterScalarMap::ModuleParameterScalarMap;
  std::shared_ptr<VModuleParameter> clone() override;
protected:
  ModuleParameter(const ModuleParameter&) = default;
};

template <>
class ModuleParameter<std::map<int, std::string>> : public ModuleParameterScalarMap<int, std::string>
{
public:
  using ModuleParameterScalarMap::ModuleParameterScalarMap;
  std::shared_ptr<VModuleParameter> clone() override;
protected:
  ModuleParameter(const ModuleParameter&) = default;
};

template <>
class ModuleParameter<std::map<std::string, int>> : public ModuleParameterScalarMap<std::string, int>
{
public:
  using ModuleParameterScalarMap::ModuleParameterScalarMap;
  std::shared_ptr<VModuleParameter> clone() override;
protected:
  ModuleParameter(const ModuleParameter&) = default;
};

template <>
class ModuleParameter<std::map<std::string, double>> : public ModuleParameterScalarMap<std::string, double>
{
public:
  using ModuleParameterScalarMap::ModuleParameterScalarMap;
  std::shared_ptr<VModuleParameter> clone() override;
protected:
  ModuleParameter(const ModuleParameter&) = default;
};

template <>
class ModuleParameter<std::map<std::string, std::string>> : public ModuleParameterScalarMap<std::string, std::string>
{
public:
  using ModuleParameterScalarMap::ModuleParameterScalarMap;
  std::shared_ptr<VModuleParameter> clone() override;
protected:
  ModuleParameter(const ModuleParameter&) = default;
};

} /* namespace anlnext */

#endif
