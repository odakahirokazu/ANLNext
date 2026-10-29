#ifndef ANLNEXT_ModuleParameter_map_H
#define ANLNEXT_ModuleParameter_map_H 1

namespace anlnext
{

template <typename Key, typename Value>
class ModuleParameterMap : public VModuleParameter
{
  using map_type = std::map<Key, Value>;
public:
  ModuleParameterMap(const std::string& name, map_type* ptr);
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
  ModuleParameterMap(const ModuleParameterMap&) = default;
  virtual map_type& __ref__();
  virtual const map_type& __ref__() const;
private:
  map_type* ptr_;
};

template <>
class ModuleParameter<std::map<int, int>> : public ModuleParameterMap<int, int>
{
public:
  using ModuleParameterMap::ModuleParameterMap;
  std::shared_ptr<VModuleParameter> clone() override;
protected:
  ModuleParameter(const ModuleParameter&) = default;
};

template <>
class ModuleParameter<std::map<int, double>> : public ModuleParameterMap<int, double>
{
public:
  using ModuleParameterMap::ModuleParameterMap;
  std::shared_ptr<VModuleParameter> clone() override;
protected:
  ModuleParameter(const ModuleParameter&) = default;
};

template <>
class ModuleParameter<std::map<int, std::string>> : public ModuleParameterMap<int, std::string>
{
public:
  using ModuleParameterMap::ModuleParameterMap;
  std::shared_ptr<VModuleParameter> clone() override;
protected:
  ModuleParameter(const ModuleParameter&) = default;
};

template <>
class ModuleParameter<std::map<std::string, int>> : public ModuleParameterMap<std::string, int>
{
public:
  using ModuleParameterMap::ModuleParameterMap;
  std::shared_ptr<VModuleParameter> clone() override;
protected:
  ModuleParameter(const ModuleParameter&) = default;
};

template <>
class ModuleParameter<std::map<std::string, double>> : public ModuleParameterMap<std::string, double>
{
public:
  using ModuleParameterMap::ModuleParameterMap;
  std::shared_ptr<VModuleParameter> clone() override;
protected:
  ModuleParameter(const ModuleParameter&) = default;
};

template <>
class ModuleParameter<std::map<std::string, std::string>> : public ModuleParameterMap<std::string, std::string>
{
public:
  using ModuleParameterMap::ModuleParameterMap;
  std::shared_ptr<VModuleParameter> clone() override;
protected:
  ModuleParameter(const ModuleParameter&) = default;
};

} /* namespace anlnext */

#endif
