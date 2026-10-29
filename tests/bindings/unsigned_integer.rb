require 'anlnext'
require 'submodule_ruby'
require 'tmpdir'

maximum = Submodule_ruby::UnsignedIntegerExample.maximum
signed_maximum = Submodule_ruby::UnsignedIntegerExample.signed_maximum
raise unless maximum >= (1 << 64) - 1

def expect_error
  begin
    yield
  rescue RuntimeError, RangeError, TypeError, ArgumentError
    return
  end
  raise 'expected error'
end

chain = ANL::AnalysisChain.new
chain.console = false
chain.num_parallels = 2
chain.chain(Submodule_ruby::UnsignedIntegerExample, :unsigned)
chain.with_parameters(address: maximum, count: 7, signed_value: -23)
chain.modify do |c|
  original = c.get_parallel_module(0, :unsigned)
  copied = c.get_parallel_module(1, :unsigned)
  param = original.get_parameter('address')
  raise unless param.type_name == 'unsigned_integer'
  raise unless original.get_parameter_value('address') == maximum
  raise unless original.get_parameter_value('signed_value') == -23
  [0, signed_maximum, signed_maximum + 1, maximum].each do |value|
    copied.set_parameter_unsigned_integer('address', value)
    raise unless copied.get_parameter('address').get_value_unsigned_integer == value
    raise unless original.get_parameter_value('address') == maximum
  end
  expect_error { param.get_value_integer }
  expect_error { param.get_value(0) }
  [-1, maximum + 1, 1.5, '123'].each do |bad|
    expect_error { original.set_parameter_unsigned_integer('address', bad) }
    raise unless original.get_parameter_value('address') == maximum
  end
  expect_error { original.set_parameter_integer('address', -1) }
  expect_error { original.set_parameter_unsigned_integer('count', maximum) }
  expect_error { original.get_parameter('count').get_value_unsigned_integer }
  expect_error { original.set_parameter('address', -1) }
  original.set_parameter('address', maximum)
  c.expose_module(:unsigned)
  [-1, maximum + 1].each do |bad|
    expect_error { c.with_parameters(address: bad) }
    raise unless original.get_parameter_value('address') == maximum
  end
end
Dir.mktmpdir do |directory|
  chain.parameters_json_filename = File.join(directory, 'unsigned.json')
  chain.parameters_json_master = false
  chain.run(0)
  cpp = JSON.parse(File.read(chain.parameters_json_filename))['application']
  %w[module_list chain1].each do |key|
    param = cpp[key].first['parameter_list'].find { |p| p['name'] == 'address' }
    raise unless param['type'] == 'unsigned_integer' && param['value'] == maximum.to_s
  end
  ruby = JSON.parse(JSON.generate(chain.parameters_to_object))['application']['module_list']
  param = ruby.first['parameter_list'].find { |p| p['name'] == 'address' }
  raise unless param['type'] == 'unsigned_integer' && param['value'] == maximum
  filename = File.join(directory, 'unsigned.rb')
  chain.make_script(output: filename)
  script = File.read(filename)
  RubyVM::InstructionSequence.compile(script)
  raise unless script.include?(maximum.to_s)
end
