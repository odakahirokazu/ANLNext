#ifndef ANLNEXT_ModuleParameter_unsigned_integer_H
#define ANLNEXT_ModuleParameter_unsigned_integer_H 1

namespace anlnext
{

template <>
class ModuleParameter<uintmax_t> : public VModuleParameter
{
public:
  ModuleParameter(const std::string& name, uintmax_t* ptr);
  std::shared_ptr<VModuleParameter> clone() override;
  std::string type_name() const override;

  void set_value(int value) override;
  void set_value(uintmax_t value) override;
  void set_value_integer(intmax_t value) override;
  void set_value_unsigned_integer(uintmax_t value) override;
  using VModuleParameter::set_value;

  int get_value(int) const override;
  uintmax_t get_value(uintmax_t) const override;
  intmax_t get_value_integer() const override;
  uintmax_t get_value_unsigned_integer() const override;
  using VModuleParameter::get_value;

  void output(std::ostream& os) const override;
  void input(std::istream& is) override;
  void __get__(void* ptr) const override;
  void __set__(const void* ptr) override;
  boost::property_tree::ptree to_property_tree() const override;

protected:
  ModuleParameter(const ModuleParameter&) = default;
  void __set_pointer__(uintmax_t* ptr) { ptr_ = ptr; }
  virtual uintmax_t& __ref__();
  virtual const uintmax_t& __ref__() const;

private:
  uintmax_t* ptr_;
};

} /* namespace anlnext */

#endif
