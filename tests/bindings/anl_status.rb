require 'anlnext'
require 'submodule_ruby'

names = %w[
  proceed proceed_with_error skip skip_with_error
  quit quit_with_error quit_all quit_all_with_error
  critical_error_to_finalize critical_error_to_terminate
  critical_error_to_finalize_from_exception critical_error_to_terminate_from_exception
]
names.each_with_index do |name, value|
  code = ANL.const_get("ANLStatus_#{name}")
  raise unless code == value
  raise unless ANL.status_to_string(code) == "AS_#{name.upcase}"
  unless name.end_with?('_from_exception')
    raise unless ANL.const_get("AS_#{name.upcase}") == code
  end
end

[
  ['proceed_with_error', ANL::AS_PROCEED], ['skip_with_error', ANL::AS_PROCEED],
  ['quit_with_error', ANL::AS_QUIT], ['quit_all_with_error', ANL::AS_QUIT],
].each do |name, expected|
  code = ANL.const_get("AS_#{name.upcase}")
  mod = Submodule_ruby::DefinitionStatusExample.new(code)
  manager = ANL::ANLManager.new
  manager.set_modules([mod])
  raise unless manager.define_analysis == expected
end
