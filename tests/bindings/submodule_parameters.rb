require 'anlnext'
require 'submodule_ruby'

def expect_error(name)
  begin
    yield
  rescue RuntimeError => error
    raise unless error.message.include?(name)
    return
  end
  raise "Expected #{name}"
end

chain = ANL::AnalysisChain.new
initializer = ANL::ModuleInitializer.new(Submodule_ruby::SubmoduleExample, :first)
initializer.with_submodule('algorithm', 'multiply')
settings = { algorithm_: {factor: 7} }
initializer.with_parameters(settings)
chain.chain_with_parameters(initializer)
settings[:algorithm_][:factor] = 99
chain.chain(Submodule_ruby::SubmoduleExample, :second)
chain.with_submodule('algorithm', 'containers')
chain.with_parameters("algorithm.factor": 11)
chain.define
chain.load_all_parameters
first = chain.get_module(:first).get_submodule('algorithm')
second = chain.get_module(:second).get_submodule('algorithm')
raise unless first.get_parameter_value('factor') == 7
raise unless second.get_parameter_value('factor') == 11

chain.with_parameters(input: 4, "algorithm.factor": 12,
                      algorithm_: {factor: 13, samples: [2, 3], weights: {'left_' => 0.5, 'a.b' => 1.5}})
raise unless second.get_parameter_value('factor') == 13
raise unless second.get_parameter_value('samples') == [2, 3]
raise unless second.get_parameter_value('weights') == {'left_' => 0.5, 'a.b' => 1.5}
raise unless chain.get_module(:second).get_parameter_value('input') == 4
raise unless first.get_parameter_value('factor') == 7
chain.with_parameters(algorithm_: {factor: 14}, "algorithm.factor": 15)
raise unless second.get_parameter_value('factor') == 15
chain.with_parameters('algorithm_' => {factor: 13})
chain.with_parameters(algorithm_: {})
raise unless second.get_parameter_value('factor') == 13
expect_error('SubModuleUndefinedError') { chain.with_parameters("missing.factor": 1) }
expect_error('SubModuleNotSetError') { chain.with_parameters("optional.factor": 1) }
expect_error('ParameterNotFound') { chain.with_parameters("algorithm.missing": 1) }
expect_error('SubModuleUndefinedError') { chain.with_parameters(missing_: {factor: 1}) }
expect_error('SubModuleNotSetError') { chain.with_parameters(optional_: {factor: 1}) }
expect_error('ParameterNotFound') { chain.with_parameters(algorithm_: {missing: 1}) }

[{algorithm_: 1}, {algorithm_: [1]}, {'_' => {}}].each do |parameters|
  begin
    chain.with_parameters(parameters)
  rescue ArgumentError
    next
  end
  raise 'Expected invalid submodule parameter group'
end

['.factor', 'algorithm.', 'algorithm.factor.extra'].each do |name|
  begin
    chain.with_parameters(name => 1)
  rescue ArgumentError
    next
  end
  raise 'Expected invalid parameter path'
end
