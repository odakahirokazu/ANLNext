#include <iostream>
#include <anl/CLIUtility.hh>

int main()
{
  using namespace anl;

  int i = 0;
  cli_read("number", &i);
  std::cout << "number is " << i << std::endl;

  cli_read("number", &i, "hex");
  std::cout << "number is " << i << std::endl;

  double d = 0;
  cli_read("value", &d, 1000.0, "km");
  std::cout << "value is " << d << " in units of m" << std::endl;

  std::vector<std::string> ss;
  cli_read("list", &ss);
  std::cout << "List: ";
  for (const auto& s: ss) { std::cout << s << " "; }
  std::cout << std::endl;

  cli_read_seq("list", &ss, "F3");
  std::cout << "List: ";
  for (const auto& s: ss) { std::cout << s << " "; }
  std::cout << std::endl;

  return 0;
}
