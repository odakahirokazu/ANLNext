import anlnext
from submodule_python import MapExample, RequirementExample

for order in ['valid', 'reversed', 'missing']:
    producer = MapExample()
    producer.set_module_id('producer')
    consumer = RequirementExample()
    consumer.set_module_id('consumer')
    manager = anlnext.ANLManagerMT(3)
    modules = [producer, consumer] if order == 'valid' else [consumer, producer]
    manager.set_modules([consumer] if order == 'missing' else modules)
    if order == 'valid':
        assert manager.Define() == anlnext.status.AS_OK
        assert manager.PreInitialize() == anlnext.status.AS_OK
    else:
        try:
            manager.Define()
        except RuntimeError as error:
            text = str(error)
            assert all(word in text for word in ['ModuleRequirementError', 'consumer', 'producer', 'before'])
        else:
            raise AssertionError('unsatisfied requirement was accepted')
