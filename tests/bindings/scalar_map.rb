require 'anlnext'
require 'submodule_ruby'

values = {
  'map_i_i' => { -1 => 3, 2147483647 => -2147483648 },
  'map_i_d' => { -1 => 0.5, 2 => 1.5 },
  'map_i_str' => { -1 => '', 2 => "a b\"c\\d" },
  'map_str_i' => { '' => 3, 'a.b' => 5 },
  'map_str_d' => { 'a b' => 0.5, 'a.b' => 1.5 },
  'map_str_str' => { '' => '', 'a b' => "q\"\\r" }
}
chain = ANL::AnalysisChain.new
chain.console = false
chain.num_parallels = 2
chain.chain(Submodule_ruby::ScalarMapExample, :maps)
chain.with_parameters(values.merge('table' => { 'row' => [3, 0.5] }))
chain.modify do |c|
  original = c.get_parallel_module(0, :maps)
  copied = c.get_parallel_module(1, :maps)
  values.each do |name, expected|
    raise unless original.get_parameter_value(name) == expected
    raise unless copied.get_parameter_value(name) == expected
    copied.set_parameter(name, {})
    raise unless copied.get_parameter_value(name) == {}
    raise unless original.get_parameter_value(name) == expected
  end
  [['map_i_i', { '1' => 3 }], ['map_i_i', { 1.5 => 3 }],
   ['map_i_i', { 2147483648 => 1 }], ['map_i_i', { 1 => 2147483648 }],
   ['map_i_i', { 1 => 1.5 }], ['map_str_i', { 1 => 2 }],
   ['map_i_d', { 1 => 'bad' }], ['map_i_str', { 1 => 2 }]].each do |name, bad|
    begin
      original.set_parameter(name, bad)
    rescue TypeError, RangeError, ArgumentError
      raise unless original.get_parameter_value(name) == values[name]
      next
    end
    raise "accepted invalid map: #{name}: #{bad}"
  end
  begin
    original.get_parameter('map_i_i').get_value_map_str_str({})
  rescue RuntimeError
    getter_failed = true
  end
  raise unless getter_failed
  raise unless original.get_parameter_value('table') == { 'row' => { 'count' => 3, 'weight' => 0.5 } }
end
chain.run(0)
parameters = chain.parameters_to_object[:application][:module_list].first[:parameter_list]
parameters.each do |param|
  next unless values.key?(param[:name])
  raise unless param[:value].map { |entry| [entry[:key], entry[:value]] }.to_h == values[param[:name]]
  raise unless param[:unit] == 10.0 if param[:name].end_with?('_d')
end
