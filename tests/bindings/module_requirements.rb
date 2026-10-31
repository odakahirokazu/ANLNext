require 'anlnext'
require 'submodule_ruby'

%w[valid reversed missing].each do |order|
  producer = Submodule_ruby::MapExample.new
  producer.set_module_id('producer')
  consumer = Submodule_ruby::RequirementExample.new
  consumer.set_module_id('consumer')
  manager = ANL::ANLManagerMT.new(3)
  modules = order == 'valid' ? [producer, consumer] : [consumer, producer]
  manager.set_modules(order == 'missing' ? [consumer] : modules)
  if order == 'valid'
    raise unless manager.define_analysis == ANL::AS_OK
    raise unless manager.pre_initialize_analysis == ANL::AS_OK
  else
    begin
      manager.define_analysis
    rescue RuntimeError => error
      raise error unless %w[ModuleRequirementError consumer producer before].all? { |word| error.message.include?(word) }
      next
    end
    raise 'unsatisfied requirement was accepted'
  end
end
