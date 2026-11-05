import anlnext
from submodule_python import SubmoduleExample, MapExample, UnsignedIntegerExample


def make_chain(module_class):
    chain = anlnext.AnalysisChain()
    chain.console = False
    chain.chain(module_class, 'target')
    return chain


def expect_error(message, operation):
    try:
        operation()
    except Exception as error:
        assert message in str(error), str(error)
        return
    raise AssertionError(f'Expected error: {message}')


def value(module, name):
    return anlnext.get_value(module.get_parameter(name))


chain = make_chain(UnsignedIntegerExample)
chain.with_parameters({'count': 1})
expect_error('Definition stage', chain.load_all_parameters)
assert chain.define() == anlnext.ANLStatus_proceed
manager = chain.anl_manager
assert chain.define() == anlnext.ANLStatus_proceed and chain.anl_manager is manager
chain.with_parameters({'count': 2})
module = chain.get_module('target')
assert value(module, 'count') == 0
chain.load_all_parameters()
assert value(module, 'count') == 2
chain.with_parameters({'count': 3})
assert value(module, 'count') == 2
chain.run(0)
assert value(module, 'count') == 3
for setter in [
    lambda: chain.with_parameters({'count': 4}),
    lambda: chain.set_parameter('count', 4),
    lambda: chain.set_module_parameter('target', 'count', 4),
    lambda: chain.set_module_parameters('target', {'count': 4}),
    lambda: chain.push_to_vector('rows', {'count': 4}),
    lambda: chain.insert_to_map('table', 'row', {'count': 4}),
    lambda: chain.with_submodule('algorithm', 'multiply'),
    lambda: chain.load_parameters_from_yaml('not-read.yaml'),
    lambda: chain.with_setter(lambda module: None),
]:
    expect_error('not allowed', setter)
module.set_parameter_integer('count', 4)
assert value(module, 'count') == 4

chain = make_chain(MapExample)
calls = []
chain.push_to_vector('rows', {'count': 7})


def first(module):
    calls.append('first')
    anlnext.set_map_parameter(module, 'map_i_i', {1: 3})
    chain.push_to_vector('rows', {'count': 13})


chain.with_setter(first)
chain.push_to_vector('rows', {'count': 11})
chain.with_setter(lambda module: calls.append('second'))
chain.insert_to_map('table', 'row', {'count': 5})
assert chain.define() == anlnext.ANLStatus_proceed
chain.load_all_parameters()
module = chain.get_module('target')
assert [row['count'] for row in value(module, 'rows')] == [7, 11, 13]
assert value(module, 'map_i_i') == {1: 3}
assert value(module, 'table')['row']['count'] == 5
assert calls == ['first', 'second']
chain.load_all_parameters()
assert len(value(module, 'rows')) == 3 and calls == ['first', 'second']

chain = make_chain(SubmoduleExample)
chain.num_parallels = 2
chain.with_submodule('algorithm', 'multiply')
chain.with_parameters({'input': 4, 'algorithm_': {'factor': 7}})
assert chain.define() == anlnext.ANLStatus_proceed
chain.load_all_parameters()
chain.with_parameters({'input': 5})


def modify(c):
    selected = c.current_module
    for index in range(2):
        module = c.get_parallel_module(index, 'target')
        assert value(module, 'input') == 5
        c.current_module = module
        c.with_parameters({'input': 10 + index, 'algorithm_': {'factor': 8 + index}})
        assert value(module, 'input') == 5
    c.current_module = selected


chain.modify(modify)
chain.run(0)
assert value(chain.get_parallel_module(0, 'target'), 'result') == 80
assert value(chain.get_parallel_module(1, 'target'), 'result') == 99

chain = make_chain(MapExample)
calls = []
chain.push_to_vector('rows', {'count': 7})
chain.with_setter(lambda module: calls.append('before'))
chain.with_parameters({'missing': 1})
chain.with_setter(lambda module: calls.append('after'))
assert chain.define() == anlnext.ANLStatus_proceed
expect_error('ParameterNotFound', chain.load_all_parameters)
for _ in range(2):
    expect_error('not allowed', chain.load_all_parameters)
expect_error('cannot start', lambda: chain.run(0))
assert value(chain.get_module('target'), 'rows') == [{'count': 7}]
assert calls == ['before']
expect_error('not allowed', lambda: chain.with_setter(lambda module: None))

chain = make_chain(UnsignedIntegerExample)
chain.with_parameters({'missing': 1})
expect_error('ParameterNotFound', lambda: chain.run(0))

chain = make_chain(SubmoduleExample)
chain.num_parallels = 2
chain.with_submodule('algorithm', 'multiply')
chain.modify(lambda c: c.with_parameters({'input': 9, 'missing': 1}))
expect_error('ParameterNotFound', lambda: chain.run(0))
assert value(chain.get_parallel_module(0, 'target'), 'input') == 9
for index in range(2):
    assert value(chain.get_parallel_module(index, 'target'), 'result') == 0

chain = make_chain(UnsignedIntegerExample)
calls = []


def reenter(module):
    calls.append('entered')
    chain.load_all_parameters()


chain.with_setter(reenter)
assert chain.define() == anlnext.ANLStatus_proceed
expect_error('already in progress', chain.load_all_parameters)
expect_error('not allowed', chain.load_all_parameters)
assert calls == ['entered']

chain = anlnext.AnalysisChain()
expect_error('No current module', lambda: chain.with_parameters({'count': 1}))

chain = make_chain(UnsignedIntegerExample)
chain.chain(UnsignedIntegerExample, 'other')
assert chain.set_parameter('count', 1) is chain
assert chain.set_parameters({'address': 9, 'signed_value': -3}) is chain
assert chain.set_module_parameter('target', 'count', 2) is chain
assert chain.current_module is chain.get_module('target')
assert chain.set_module_parameters('target', {'count': 3}) is chain
chain.with_parameters({'count': 4})
chain.set_module_parameters('other', {'address': 11})
chain.with_setter(lambda module: module.set_parameter_integer('count', 5))
expect_error('not registered', lambda: chain.set_module_parameter('missing', 'count', 0))
expect_error('not registered', lambda: chain.set_module_parameters('missing', {'count': 0}))
assert chain.current_module is chain.get_module('other')
assert chain.define() == anlnext.ANLStatus_proceed
assert value(chain.get_module('target'), 'count') == 0
chain.load_all_parameters()
assert value(chain.get_module('target'), 'count') == 4
assert value(chain.get_module('other'), 'count') == 5
assert value(chain.get_module('other'), 'address') == 11
assert value(chain.get_module('other'), 'signed_value') == -3
