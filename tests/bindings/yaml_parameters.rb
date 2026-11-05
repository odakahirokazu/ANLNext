require 'anlnext'
require 'submodule_ruby'
require 'tmpdir'

def make_chain
  chain = ANL::AnalysisChain.new
  chain.console = false
  chain.num_parallels = 3
  [:first, :second].each do |id|
    chain.chain(Submodule_ruby::SubmoduleExample, id)
    chain.with_submodule('algorithm', 'containers')
  end
  chain.chain(Submodule_ruby::MapExample, :maps)
  chain.chain(Submodule_ruby::UnsignedIntegerExample, :unsigned)
  chain
end

filename = File.join(__dir__, 'yaml_parameters.yaml')
chain = make_chain
current = chain.current_module
chain.with_parameters(count: 1)
chain.load_parameters_from_yaml(filename)
raise unless chain.current_module.equal?(current)
chain.with_parameters(count: 5)
expected_maps = YAML.safe_load_file(filename, aliases: true)['maps']
manager = chain.define
chain.load_all_parameters
raise unless manager.pre_initialize_analysis == ANL::AS_OK
3.times do |index|
  [:first, :second].each do |id|
    mod = chain.get_parallel_module(index, id)
    algorithm = mod.get_submodule('algorithm')
    expected_factor = (id == :first) ? 7 : 11
    raise unless algorithm.get_parameter_value('factor') == expected_factor
    raise unless algorithm.get_parameter_value('samples') == [2, 3]
    raise unless algorithm.get_parameter_value('weights') == {'left_' => 0.5, 'a.b' => 1.5}
    raise unless algorithm.get_parameter_value('enabled') == (id == :first)
    raise unless algorithm.get_parameter_value('label') == 'on'
  end
  maps = chain.get_parallel_module(index, :maps)
  expected_maps.each do |name, expected|
    actual = maps.get_parameter_value(name)
    # SWIG returns std::string values as binary strings.
    expected = expected.transform_values(&:b) if name == 'map_str_str'
    expected = expected.map(&:b) if name == 'vector_str'
    raise "#{name}: #{actual.inspect} != #{expected.inspect}" unless actual == expected
  end
  unsigned = chain.get_parallel_module(index, :unsigned)
  raise unless unsigned.get_parameter_value('address') == Submodule_ruby::UnsignedIntegerExample.maximum
  raise unless unsigned.get_parameter_value('signed_value') == -9223372036854775808
  raise unless unsigned.get_parameter_value('count') == 5
end

app = ANL::ANLApp.new
app.chain(Submodule_ruby::UnsignedIntegerExample, :unsigned)
Dir.mktmpdir do |directory|
  file = File.join(directory, 'parameters.yaml')
  File.write(file, "unsigned: {count: 17}\n")
  app.define
  app.load_parameters_from_yaml(file)
  raise unless app.get_module(:unsigned).get_parameter_value('count') == 0
  app.load_all_parameters
  raise unless app.get_module(:unsigned).get_parameter_value('count') == 17

  ['', '[]', 'null', 'unsigned:', '1: {}', 'unsigned: {1: 7}',
   "unsigned: {count: 9}\nmissing: {}", 'unsigned: {', '!ruby/object:Object {}',
   "unsigned: {}\n---\nunsigned: {}"].each do |text|
    invalid = make_chain
    selected = invalid.current_module
    File.write(file, text)
    begin
      invalid.load_parameters_from_yaml(file)
    rescue ArgumentError, Psych::Exception
      raise unless invalid.current_module.equal?(selected)
      next
    end
    raise "Accepted invalid YAML: #{text}"
  end

  invalid = make_chain
  selected = invalid.current_module
  File.write(file, 'first: {algorithm_: 1}')
  begin
    invalid.load_parameters_from_yaml(file)
  rescue ArgumentError
    raise unless invalid.current_module.equal?(selected)
    group_failed = true
  end
  raise unless group_failed

  invalid = make_chain
  File.write(file, 'first: {algorithm_: {missing: 1}}')
  invalid.load_parameters_from_yaml(file)
  invalid.define
  begin
    invalid.load_all_parameters
  rescue RuntimeError => error
    raise unless error.message.include?('ParameterNotFound')
    value_failed = true
  end
  raise unless value_failed
end
