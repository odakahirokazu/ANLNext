#!/usr/bin/env ruby

require 'anlnext'
require 'submoduleExample'

chain = ANL::AnalysisChain.new
chain.console = false
chain.num_parallels = Integer(ARGV.fetch(0, '3'))

chain.chain(SubmoduleExample::AlgorithmModule, :multiply)
chain.with_submodule('algorithm', 'multiply')
chain.with_parameters(input: 4.0, "algorithm.factor": 7.0)

chain.chain(SubmoduleExample::AlgorithmModule, :offset)
chain.with_submodule('algorithm', 'offset')
chain.with_parameters(input: 4.0, algorithm_: {offset: 3.0})

chain.run(10)

{ multiply: ['factor', 7.0, 280.0], offset: ['offset', 3.0, 70.0] }.each do |id, expected|
  name, value, total = expected
  mod = chain.get_module(id)
  algorithm = mod.get_submodule('algorithm')
  puts "#{id}: #{algorithm.submodule_name}"
  algorithm.print_parameters
  puts "#{name} = #{algorithm.get_parameter_value(name)}"
  puts "total = #{mod.get_parameter_value('total')}"
  raise 'Unexpected total' unless mod.get_parameter_value('total') == total
  chain.num_parallels.times do |index|
    copy = chain.get_parallel_module(index, id).get_submodule('algorithm')
    raise 'Unexpected parameter' unless copy.get_parameter_value(name) == value
  end
end
