require 'anlnext'
require 'submodule_ruby'

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
chain.run(6)
3.times do |index|
  mod = chain.get_parallel_module(index, :example)
  raise unless value(mod, 'result') == 4 * (7 + index)
end
