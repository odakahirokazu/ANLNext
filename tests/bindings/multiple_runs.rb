require 'anlnext'
require 'submodule_ruby'

undefined_loop_index = ANL::BasicModule.undefined_loop_index
undefined_run_index = ANL::ANLManager.undefined_run_index
raise unless undefined_run_index == ANL::BasicModule.undefined_run_index && undefined_run_index == undefined_loop_index
[1, 3].each do |parallels|
  manager = parallels == 1 ? ANL::ANLManager.new : ANL::ANLManagerMT.new(parallels)
  mod = Submodule_ruby::MapExample.new
  mod.set_module_id('module')
  manager.set_modules([mod])
  manager.set_print_parallel_modules(false)
  raise unless manager.run_index == undefined_run_index && mod.get_run_index == undefined_run_index
  raise unless manager.define_analysis == ANL::AS_OK
  raise unless manager.pre_initialize_analysis == ANL::AS_OK
  raise unless manager.initialize_analysis == ANL::AS_OK
  raise unless manager.run_index == undefined_run_index
  [5, 0, 2].each_with_index do |count, run_index|
    raise unless manager.run_analysis(count, false) == ANL::AS_OK
    raise unless manager.run_index == run_index
    parallels.times do |chain_index|
      copy = manager.access_to_module(chain_index, 'module')
      raise unless copy.get_run_index == run_index
      raise unless copy.get_loop_index == undefined_loop_index || copy.get_loop_index < count
      raise unless count != 0 || copy.get_loop_index == undefined_loop_index
    end
  end
  raise unless manager.finalize_analysis == ANL::AS_OK
  raise unless manager.run_index == 2
end
