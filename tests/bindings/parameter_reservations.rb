require 'anlnext'
require 'submodule_ruby'

def make_chain(klass)
  chain = ANL::AnalysisChain.new
  chain.console = false
  chain.chain(klass, :target)
  chain
end

def expect_error(message)
  begin
    yield
  rescue StandardError => error
    raise unless error.message.include?(message)
    return
  end
  raise "Expected error: #{message}"
end

chain = make_chain(Submodule_ruby::UnsignedIntegerExample)
chain.with_parameters(count: 1)
expect_error('Definition stage') { chain.load_all_parameters }
manager = chain.define
raise unless chain.define.equal?(manager)
chain.with_parameters(count: 2)
mod = chain.get_module(:target)
raise unless mod.get_parameter_value('count') == 0
chain.load_all_parameters
raise unless mod.get_parameter_value('count') == 2
chain.with_parameters(count: 3)
raise unless chain.parameters_to_object[:application][:module_list].first[:parameter_list]
                  .find{|parameter| parameter[:name] == 'count' }[:value] == 2
chain.run(0)
raise unless mod.get_parameter_value('count') == 3
raise unless chain.finalization_done?
[
  lambda{ chain.with_parameters(count: 4) },
  lambda{ chain.set_parameter('count', 4) },
  lambda{ chain.set_module_parameter(:target, 'count', 4) },
  lambda{ chain.set_module_parameters(:target, count: 4) },
  lambda{ chain.push_to_vector('rows', count: 4) },
  lambda{ chain.insert_to_map('table', 'row', count: 4) },
  lambda{ chain.with_submodule('algorithm', 'multiply') },
  lambda{ chain.load_parameters_from_yaml('not-read.yaml') },
  lambda{ chain.with_parameters{} },
].each do |setter|
  expect_error('not allowed') { setter.call }
end
mod.set_parameter_integer('count', 4)
raise unless mod.get_parameter_value('count') == 4

chain = make_chain(Submodule_ruby::MapExample)
calls = []
chain.push_to_vector('rows', count: 7)
chain.with_parameters do |mod|
  calls << :first
  mod.set_parameter('map_i_i', {1 => 3})
  chain.push_to_vector('rows', count: 13)
end
chain.push_to_vector('rows', count: 11)
chain.with_parameters{ calls << :second }
chain.insert_to_map('table', 'row', count: 5)
chain.define
chain.load_all_parameters
mod = chain.get_module(:target)
raise unless mod.get_parameter_value('rows').map{|row| row['count'] } == [7, 11, 13]
raise unless mod.get_parameter_value('map_i_i') == {1 => 3}
raise unless mod.get_parameter_value('table')['row']['count'] == 5
raise unless calls == [:first, :second]
chain.load_all_parameters
raise unless mod.get_parameter_value('rows').size == 3 && calls == [:first, :second]

chain = make_chain(Submodule_ruby::SubmoduleExample)
chain.num_parallels = 2
chain.with_submodule('algorithm', 'multiply')
chain.with_parameters(input: 4, algorithm_: {factor: 7})
chain.define
chain.load_all_parameters
chain.with_parameters(input: 5)
chain.modify do |c|
  selected = c.current_module
  2.times do |index|
    mod = c.get_parallel_module(index, :target)
    raise unless mod.get_parameter_value('input') == 5
    c.modify_parameters(index, :target, input: 10 + index, algorithm_: {factor: 8 + index})
    raise unless mod.get_parameter_value('input') == 5
  end
  raise unless c.current_module.equal?(selected)
end
chain.run(0)
raise unless chain.get_parallel_module(0, :target).get_parameter_value('result') == 80
raise unless chain.get_parallel_module(1, :target).get_parameter_value('result') == 99

chain = make_chain(Submodule_ruby::MapExample)
calls = []
chain.push_to_vector('rows', count: 7)
chain.with_parameters{ calls << :before }
chain.with_parameters(missing: 1)
chain.with_parameters{ calls << :after }
chain.define
expect_error('ParameterNotFound') { chain.load_all_parameters }
2.times { expect_error('not allowed') { chain.load_all_parameters } }
expect_error('cannot start') { chain.run(0) }
raise unless chain.get_module(:target).get_parameter_value('rows') == [{'count' => 7}]
raise unless calls == [:before]
expect_error('not allowed') { chain.with_parameters{} }
chain.clear
chain.chain(Submodule_ruby::UnsignedIntegerExample, :reset)
chain.with_parameters(count: 9)
chain.run(0)
raise unless chain.get_module(:reset).get_parameter_value('count') == 9

chain = make_chain(Submodule_ruby::UnsignedIntegerExample)
chain.with_parameters(missing: 1)
expect_error('ParameterNotFound') { chain.run(0) }

chain = make_chain(Submodule_ruby::SubmoduleExample)
chain.num_parallels = 2
chain.with_submodule('algorithm', 'multiply')
chain.modify do |c|
  c.with_parameters(input: 9, missing: 1)
end
expect_error('ParameterNotFound') { chain.run(0) }
raise unless chain.get_parallel_module(0, :target).get_parameter_value('input') == 9
2.times do |index|
  raise unless chain.get_parallel_module(index, :target).get_parameter_value('result') == 0
end

chain = make_chain(Submodule_ruby::UnsignedIntegerExample)
calls = []
chain.with_parameters do
  calls << :entered
  chain.load_all_parameters
end
chain.define
expect_error('already in progress') { chain.load_all_parameters }
expect_error('not allowed') { chain.load_all_parameters }
raise unless calls == [:entered]

chain = ANL::AnalysisChain.new
expect_error('No current module') { chain.with_parameters(count: 1) }

chain = make_chain(Submodule_ruby::UnsignedIntegerExample)
chain.chain(Submodule_ruby::UnsignedIntegerExample, :other)
chain.set_parameter('count', 1)
chain.set_parameters(address: 9, signed_value: -3)
chain.set_module_parameter(:target, 'count', 2)
raise unless chain.current_module.equal?(chain.get_module(:target))
chain.set_module_parameters(:target, count: 3)
chain.with_parameters(count: 4)
block_called = false
chain.set_module_parameters(:other, address: 11) do |mod|
  block_called = true
  mod.set_parameter_integer('count', 5)
end
raise if block_called
expect_error('not registered') { chain.set_module_parameter(:missing, 'count', 0) }
expect_error('not registered') { chain.set_module_parameters(:missing, count: 0) }
raise unless chain.current_module.equal?(chain.get_module(:other))
chain.define
raise unless chain.get_module(:target).get_parameter_value('count') == 0
chain.load_all_parameters
raise unless chain.get_module(:target).get_parameter_value('count') == 4
raise unless chain.get_module(:other).get_parameter_value('count') == 5
raise unless chain.get_module(:other).get_parameter_value('address') == 11
raise unless chain.get_module(:other).get_parameter_value('signed_value') == -3
raise unless block_called

chain = ANL::AnalysisChain.new
initializer = ANL::ModuleInitializer.new(Submodule_ruby::UnsignedIntegerExample, :initial)
chain.current_module = initializer
chain.set_parameter('count', 1)
chain.set_parameters(count: 2) do |mod|
  raise unless mod.get_parameter_value('count') == 2
  mod.set_parameter_integer('count', 3)
end
chain.with_parameters(count: 4)
chain.apply_setting { |mod| mod.set_parameter_integer('count', 5) }
chain.chain_with_parameters(initializer)
chain.define
chain.load_all_parameters
raise unless chain.get_module(:initial).get_parameter_value('count') == 5

app = ANL::ANLApp.new
app.chain(Submodule_ruby::UnsignedIntegerExample, :target)
app.set_parameter('count', 1)
app.set_parameters(count: 2)
app.set_module_parameter(:target, 'count', 3)
app.set_module_parameters(:target, count: 4)
app.with_parameters(count: 5){ |mod| mod.set_parameter_integer('count', 6) }
app.apply_setting { |mod| mod.set_parameter_integer('count', 7) }
app.define
app.load_all_parameters
raise unless app.get_module(:target).get_parameter_value('count') == 7

chain = make_chain(Submodule_ruby::UnsignedIntegerExample)
chain.num_parallels = 2
chain.with_parameters(count: 2)
manager = chain.define
communication_done = false
analysis_done = false
manager.define_singleton_method(:do_interactive_comunication) do
  raise unless chain.get_module(:target).get_parameter_value('count') == 2
  chain.with_parameters(count: 3)
  communication_done = true
  ANL::AS_PROCEED
end
manager.define_singleton_method(:do_interactive_analysis) do
  2.times do |index|
    raise unless chain.get_parallel_module(index, :target).get_parameter_value('count') == 3
  end
  analysis_done = true
  ANL::AS_PROCEED
end
chain.run_interactive
raise unless communication_done && analysis_done && chain.finalization_done?
