#include "BasicModule.hh"

#include <iostream>
#include <stdexcept>

using namespace anlnext;

struct Interface {
  virtual ~Interface() = default;
  virtual int value() const = 0;
};

class Target : public BasicModule, public Interface {
public:
  using BasicModule::set_access_permission;
  int value() const override { return 17; }
};

class Client : public BasicModule {
public:
  using ModuleAccessInterface::request_module_interface;
  using ModuleAccessInterface::request_module_interface_non_const;
};

template <typename T>
void verify_requests(const Client& client, const std::string& name, T& target,
                     bool readable, bool writable)
{
  const T* read_only = &target;
  T* non_const = &target;
  client.request_module_interface(name, &read_only);
  client.request_module_interface_non_const(name, &non_const);
  if (read_only != (readable ? &target : nullptr) ||
      non_const != (writable ? &target : nullptr)) {
    throw std::runtime_error("interface request returned an unexpected pointer");
  }
  if (read_only && read_only->value() != 17) {
    throw std::runtime_error("interface request returned an unusable pointer");
  }
}

int main()
{
  try {
    Target target;
    BasicModule incompatible;
    Client client;
    ModuleAccess access;
    access.register_module("target", &target);
    access.register_module("incompatible", &incompatible);
    client.set_module_access(&access);

    for (auto permission: {ModuleAccess::Permission::full_access,
                           ModuleAccess::Permission::read_only_access,
                           ModuleAccess::Permission::no_access}) {
      target.set_access_permission(permission);
      const bool readable = permission != ModuleAccess::Permission::no_access;
      const bool writable = permission == ModuleAccess::Permission::full_access;
      verify_requests<Interface>(client, "target", target, readable, writable);
      verify_requests<Target>(client, "target", target, readable, writable);
    }
    for (const auto* name: {"missing", "incompatible"}) {
      verify_requests<Interface>(client, name, target, false, false);
      verify_requests<Target>(client, name, target, false, false);
    }
  }
  catch (const ANLException& ex) {
    std::cerr << ex.to_string() << '\n';
    return 1;
  }
  catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
