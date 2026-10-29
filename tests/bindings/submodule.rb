require 'anlnext'
require 'submodule_ruby'
require 'tmpdir'

def expect_error(text)
  begin
    yield
  rescue RuntimeError => error
    raise error unless error.message.include?(text)
    return
  end
  raise 'expected RuntimeError'
end

def value(mod, name)
  mod.get_parameter(name).get_value(0)
end

chain = ANL::AnalysisChain.new
chain.console = false
chain.num_parallels = 3
chain.chain(Submodule_ruby::SubmoduleExample, :example)
chain.with_parameters do |mod|
  raise unless mod.is_submodule_defined('algorithm')
  raise if mod.is_submodule_set('algorithm')
  expect_error('SubModuleNotSetError') { mod.get_submodule('algorithm') }
  expect_error('SubModuleUndefinedError') { mod.get_submodule('missing') }
  expect_error('Unknown algorithm') { mod.set_submodule_by_key('algorithm', 'unknown') }
  mod.set_submodule_by_key('algorithm', 'multiply')
  mod.get_submodule('algorithm').set_parameter_integer('factor', 99)
  mod.set_submodule_by_key('algorithm', 'multiply')
  algorithm = mod.get_submodule('algorithm')
  raise unless value(algorithm, 'factor') == 2
  algorithm.set_parameter_integer('factor', 7)
end
chain.with_parameters(input: 4)
chain.modify do |c|
  3.times do |index|
    mod = c.get_parallel_module(index, :example)
    algorithm = mod.get_submodule('algorithm')
    raise unless value(algorithm, 'factor') == 7
    raise if mod.is_submodule_set('optional')
    expect_error('SubModuleConfigurationError') { mod.set_submodule_by_key('algorithm', 'multiply') }
    algorithm.set_parameter_integer('factor', 7 + index)
  end
end
Dir.mktmpdir do |directory|
  chain.parameters_json_filename = File.join(directory, 'parameters.json')
  chain.parameters_json_master = false
  chain.run(6)
  cpp = JSON.parse(File.read(chain.parameters_json_filename))['application']
  ruby = JSON.parse(JSON.generate(chain.parameters_to_object()))['application']
  normalize = lambda do |entry|
    copy = Marshal.load(Marshal.dump(entry))
    copy.fetch('parameter_list', []).each { |p| p['value'] = p['value'].to_s }
    copy
  end
  3.times do |index|
    mod = chain.get_parallel_module(index, :example)
    ruby_module = JSON.parse(JSON.generate(mod.parameters_to_object()))
    cpp_module = cpp[index.zero? ? 'module_list' : "chain#{index}"].first
    ruby_entries = ruby_module['submodule_list'].map { |e| normalize.call(e) }
    cpp_entries = cpp_module['submodule_list'].map { |e| normalize.call(e) }
    raise unless ruby_entries == cpp_entries
    algorithm = ruby_module['submodule_list'].find { |e| e['registered_name'] == 'algorithm' }
    raise unless algorithm['name'] == 'ExampleAlgorithm' && algorithm['version'] == '1.0'
    raise unless algorithm['state'] == 'set' && algorithm['parameter_list'].first['value'] == 7 + index
    optional = ruby_module['submodule_list'].find { |e| e['registered_name'] == 'optional' }
    raise unless optional == { 'registered_name' => 'optional', 'state' => 'unset' }
  end
  raise unless ruby['module_list'].first['submodule_list'] ==
               JSON.parse(JSON.generate(chain.get_module(:example).parameters_to_object()))['submodule_list']
  raise if ANL::BasicModule.new.parameters_to_object.key?(:submodule_list)
  raise unless chain.get_module(:example).get_submodule('algorithm').get_parameter_value(:factor) == 7
end
3.times do |index|
  mod = chain.get_parallel_module(index, :example)
  raise unless value(mod, 'result') == 4 * (7 + index)
end
