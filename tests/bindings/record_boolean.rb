require 'anlnext'
require 'submodule_ruby'
require 'tmpdir'

chain = ANL::AnalysisChain.new
chain.console = false
chain.num_parallels = 2
chain.chain(Submodule_ruby::MapExample, :records)
[['yes', 7, true], ['no', 13, false]].each do |key, count, enabled|
  values = { count: count, enabled: enabled }
  chain.insert_to_map('flag_table', key, values)
  chain.push_to_vector('flags', values)
end
chain.modify do |c|
  expected_table = { 'yes' => { 'count' => 7, 'enabled' => true },
                     'no' => { 'count' => 13, 'enabled' => false } }
  expected_rows = [expected_table['yes'], expected_table['no']]
  original = c.get_parallel_module(0, :records)
  copied = c.get_parallel_module(1, :records)
  [original, copied].each do |mod|
    raise unless mod.get_parameter_value('flag_table') == expected_table
    raise unless mod.get_parameter_value('flags') == expected_rows
  end
  extra = { 'count' => 23, 'enabled' => true }
  copied.insert_to_map('flag_table', 'extra') do |registry|
    extra.each { |name, value| registry.set_value_element(name, value) }
  end
  copied.push_to_vector('flags') do |registry|
    extra.each { |name, value| registry.set_value_element(name, value) }
  end
  raise unless copied.get_parameter_value('flag_table')['extra'] == extra
  raise unless copied.get_parameter_value('flags').last == extra
  raise unless original.get_parameter_value('flag_table') == expected_table
  raise unless original.get_parameter_value('flags') == expected_rows
end
chain.run(0)

mod = chain.get_module(:records)
mod.expose_parameter('flag_table')
mod.set_value_element('enabled', false)
mod.expose_parameter('flags')
mod.set_value_element('enabled', true)
Dir.mktmpdir do |directory|
  filename = File.join(directory, 'boolean.rb')
  chain.make_script(output: filename)
  script = File.read(filename)
  RubyVM::InstructionSequence.compile(script)
  raise unless script.include?('"enabled" => true,') && script.include?('"enabled" => false,')
end
