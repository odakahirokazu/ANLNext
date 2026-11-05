require 'anlnext'
require 'submodule_ruby'

def expect_error
  begin
    yield
  rescue TypeError, RuntimeError, RangeError, ArgumentError
    return
  end
  raise 'Expected an invalid vector setting to fail'
end

chain = ANL::AnalysisChain.new
chain.chain(Submodule_ruby::MapExample, :vectors)
empty = {vector_i: [], vector_d: [], vector_str: []}
chain.set_parameters(empty)
chain.define
mod = chain.get_module(:vectors)
defaults = {vector_i: [1, 2, 3], vector_d: [1.0, 2.0], vector_str: ['default']}
defaults.each { |name, expected| raise unless mod.get_parameter_value(name) == expected }
chain.load_all_parameters
empty.each_key { |name| raise unless mod.get_parameter_value(name) == [] }

values = {vector_i: [7, -3], vector_d: [1, 2.5], vector_str: ['a b', "雪\"\\\0"]}
chain.with_parameters(values)
chain.load_all_parameters
values.each do |name, expected|
  expected = expected.map(&:b) if name == :vector_str
  raise unless mod.get_parameter_value(name) == expected
  mod.set_parameter(name, [])
  raise unless mod.get_parameter_value(name) == []
  mod.set_parameter(name, expected)
end

chain.set_parameter('vector_d', [1, 2])
chain.load_all_parameters
raise unless mod.get_parameter_value('vector_d') == [1.0, 2.0]
mod.set_parameter('vector_d', values[:vector_d])

[
  ['vector_i', [1.5]], ['vector_i', ['7']], ['vector_i', [0, 2147483648]],
  ['vector_i', [nil]], ['vector_d', ['bad']], ['vector_str', [7]],
].each do |name, bad|
  before = mod.get_parameter_value(name)
  expect_error { mod.set_parameter(name, bad) }
  raise unless mod.get_parameter_value(name) == before
end

mod.set_parameter('map_i_i', {1 => 3})
expect_error { mod.set_parameter('map_i_i', []) }
raise unless mod.get_parameter_value('map_i_i') == {1 => 3}
mod.push_to_vector('rows') { |registry| registry.set_value_element('count', 7) }
expect_error { mod.set_parameter('rows', []) }
raise unless mod.get_parameter_value('rows') == [{'count' => 7}]
expect_error { mod.set_parameter('missing', []) }
chain.set_parameter('missing', [])
expect_error { chain.load_all_parameters }
