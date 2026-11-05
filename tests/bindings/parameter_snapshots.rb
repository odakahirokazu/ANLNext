require 'anlnext'
require 'submodule_ruby'

def verify_values(mod)
  raise unless mod.get_parameter_value('vector_i') == [1, 2]
  raise unless mod.get_parameter_value('vector_str') == ['before']
  raise unless mod.get_parameter_value('map_str_str') == {'tag' => 'before'}
  raise unless mod.get_parameter_value('rows') == [{'count' => 7}, {'count' => 11}, {'count' => 23}]
  table = mod.get_parameter_value('table')
  raise unless table['first'] == {'count' => 3, 'weight' => 0.5}
  raise unless table['second'] == {'count' => 13, 'weight' => 1.5}
  raise unless table['nested'] == {'count' => 19, 'weight' => 0.75}
end

chain = ANL::AnalysisChain.new
chain.console = false
chain.num_parallels = 2
chain.chain(Submodule_ruby::MapExample, :data)
name = +'vector_i'
values = [1, 2]
chain.set_parameter(name, values)
name.replace('missing')
values << 3

text = +'before'
settings = {vector_str: [text], map_str_str: {'tag' => text}}
chain.with_parameters(settings)
text.replace('after')
settings[:vector_str] << 'extra'
settings[:map_str_str]['tag'] = 'replaced'

vector_name = +'rows'
row = {count: 7}
chain.push_to_vector(vector_name, row)
row[:count] = 11
chain.push_to_vector(vector_name, row)
row[:count] = 99
vector_name.replace('missing')

map_name = +'table'
key = +'first'
record = {count: 3, weight: 0.5}
chain.insert_to_map(map_name, key, record)
key.replace('second')
record[:count] = 13
record[:weight] = 1.5
chain.insert_to_map(map_name, key, record)
key.replace('changed')
map_name.replace('missing')
record[:count] = 99

elements = [19, 0.75]
nested = {'nested' => elements}
chain.set_parameter('table', nested)
elements[0] = 99
nested.clear

late = {count: 17}
chain.with_parameters do |mod|
  mod.push_to_vector('rows') { |registry| registry.set_value_element('count', late[:count]) }
end
late[:count] = 23
chain.define
chain.load_all_parameters
verify_values(chain.get_module(:data))
chain.modify { |c| 2.times { |index| verify_values(c.get_parallel_module(index, :data)) } }
chain.run(0)

chain = ANL::AnalysisChain.new
chain.chain(Submodule_ruby::SubmoduleExample, :worker)
name = +'algorithm'
key = +'containers'
chain.with_submodule(name, key)
name.replace('missing')
key.replace('missing')
label = +'before'
samples = [3, 5]
weights = {'left' => 0.5}
chain.set_parameters(algorithm_: {label: label, samples: samples, weights: weights})
label.replace('after')
samples << 7
weights['left'] = 9.0
chain.define
chain.load_all_parameters
algorithm = chain.get_module(:worker).get_submodule('algorithm')
raise unless algorithm.get_parameter_value('label') == 'before'
raise unless algorithm.get_parameter_value('samples') == [3, 5]
raise unless algorithm.get_parameter_value('weights') == {'left' => 0.5}

initializer = ANL::ModuleInitializer.new(Submodule_ruby::MapExample, :data)
name = +'vector_i'
values = [1, 2]
initializer.set_parameter(name, values)
name.replace('missing')
values << 3
text = +'before'
elements = [19, 0.75]
settings = {vector_str: [text], map_str_str: {'tag' => text}, table: {'nested' => elements}}
initializer.with_parameters(settings)
text.replace('after')
elements[0] = 99
settings.clear
row = {count: 7}
initializer.push_to_vector('rows', row)
row[:count] = 11
initializer.push_to_vector('rows', row)
row[:count] = 99
record = {count: 3, weight: 0.5}
initializer.insert_to_map('table', 'first', record)
record[:count] = 13
record[:weight] = 1.5
initializer.insert_to_map('table', 'second', record)
record[:count] = 99
late = {count: 17}
initializer.with_parameters do |mod|
  mod.push_to_vector('rows') { |registry| registry.set_value_element('count', late[:count]) }
end
late[:count] = 23
chain = ANL::AnalysisChain.new
chain.chain_with_parameters(initializer)
chain.define
chain.load_all_parameters
verify_values(chain.get_module(:data))

initializer = ANL::ModuleInitializer.new(Submodule_ruby::SubmoduleExample, :worker)
name = +'algorithm'
key = +'containers'
initializer.with_submodule(name, key)
name.replace('missing')
key.replace('missing')
settings = {algorithm_: {samples: [3, 5]}}
initializer.set_parameters(settings)
settings[:algorithm_][:samples] << 7
chain = ANL::AnalysisChain.new
chain.chain_with_parameters(initializer)
chain.define
chain.load_all_parameters
raise unless chain.get_module(:worker).get_submodule('algorithm').get_parameter_value('samples') == [3, 5]

class VectorProbe < ANL::BasicModule
  attr_reader :coordinates
  def set_parameter(name, *coordinates)
    @coordinates = coordinates
  end
end

chain = ANL::AnalysisChain.new
chain.chain(VectorProbe, :probe)
vector = ANL::Vector.new(1.0, 2.0)
chain.set_parameter('position', vector)
vector.set(9.0, 9.0, 9.0)
chain.define
chain.load_all_parameters
raise unless chain.get_module(:probe).coordinates == [1.0, 2.0]

initializer = ANL::ModuleInitializer.new(VectorProbe, :probe)
vector = ANL::Vector.new(1.0, 2.0, 3.0)
initializer.set_parameter('position', vector)
vector.set(9.0, 9.0)
chain = ANL::AnalysisChain.new
chain.chain_with_parameters(initializer)
chain.define
chain.load_all_parameters
raise unless chain.get_module(:probe).coordinates == [1.0, 2.0, 3.0]
