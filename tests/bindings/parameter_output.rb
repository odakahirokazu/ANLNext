require 'anlnext'
require 'submodule_ruby'
require 'tmpdir'
require 'json'

%w[table rows].each do |name|
  mod = Submodule_ruby::MapExample.new
  manager = ANL::ANLManager.new
  manager.set_modules([mod])
  raise unless manager.define_analysis == ANL::AS_PROCEED
  mod.expose_parameter(name)
  mod.set_value_element('count', 3)
  mod.set_map_key('stored.key')
  mod.insert_to_container
  key = (name == 'table') ? 'stored.key' : 0
  parameter = mod.get_parameter(name)
  mod.set_map_key('edited')
  mod.set_value_element('count', 41)
  buffer = parameter.value_element_info(0)
  raise unless buffer.get_value(0) == 41
  raise unless parameter.value_string.include?('count: 3')
  raise unless buffer.get_value(0) == 41
  stored = mod.get_parameter_value(name)
  raise unless stored[key]['count'] == 3
  raise unless parameter.to_object[:value]
  raise unless buffer.get_value(0) == 41
  snapshots = parameter.value_elements_from_container(key)
  element = snapshots[0]
  raise unless element.to_value_object == 3
  raise unless element.to_object == { name: 'count', type: 'int', value: 3 }
  raise unless element.to_object(children_into_hash: true)[:value] == 3
  raise unless element.to_value_object(children_into_hash: false, value_element_to_object: true) == 3
  snapshots = nil
  GC.start
  raise unless element.to_value_object == 3
  Dir.mktmpdir do |directory|
    path = File.join(directory, 'parameters.json')
    manager.parameters_to_json(path)
    raise unless JSON.parse(File.read(path))['application']['module_list'][0]['parameter_list']
  end
  raise unless buffer.get_value(0) == 41
  mod.insert_to_container
  stored = mod.get_parameter_value(name)
  raise unless stored[(name == 'table') ? 'edited' : -1]['count'] == 41
  raise unless buffer.get_value(0) == 0
  GC.start
  raise unless element.to_object[:value] == 3
  begin
    parameter.value_elements_from_container((name == 'table') ? 'missing' : 99)
  rescue RuntimeError
    missing = true
  end
  raise unless missing && buffer.get_value(0) == 0
end
