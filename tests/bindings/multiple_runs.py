import anlnext
from submodule_python import MapExample

undefined_loop_index = anlnext.BasicModule.undefined_loop_index
undefined_run_index = anlnext.ANLManager.undefined_run_index
assert undefined_run_index == anlnext.BasicModule.undefined_run_index == undefined_loop_index
for parallels in [1, 3]:
    manager = anlnext.ANLManager() if parallels == 1 else anlnext.ANLManagerMT(parallels)
    module = MapExample()
    module.set_module_id('module')
    manager.set_modules([module])
    manager.set_print_parallel_modules(False)
    assert manager.run_index() == undefined_run_index
    assert module.get_run_index() == undefined_run_index
    assert manager.define_analysis() == anlnext.ANLStatus_ok
    assert manager.pre_initialize_analysis() == anlnext.ANLStatus_ok
    assert manager.initialize_analysis() == anlnext.ANLStatus_ok
    assert manager.run_index() == undefined_run_index
    for run_index, count in enumerate([5, 0, 2]):
        assert manager.run_analysis(count, False) == anlnext.ANLStatus_ok
        assert manager.run_index() == run_index
        for chain_index in range(parallels):
            copy = manager.access_to_module(chain_index, 'module')
            assert copy.get_run_index() == run_index
            assert copy.get_loop_index() == undefined_loop_index or copy.get_loop_index() < count
            if count == 0:
                assert copy.get_loop_index() == undefined_loop_index
    assert manager.finalize_analysis() == anlnext.ANLStatus_ok
    assert manager.run_index() == 2
