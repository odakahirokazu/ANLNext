import anlnext
from submodule_python import MapExample, SubmoduleExample


def value(module, name):
    return anlnext.get_value(module.get_parameter(name))


def verify_values(module):
    assert list(value(module, 'vector_i')) == [1, 2]
    assert list(value(module, 'vector_str')) == ['before']
    assert value(module, 'map_str_str') == {'tag': 'before'}
    assert value(module, 'rows') == [{'count': 7}, {'count': 11}, {'count': 23}]
    table = value(module, 'table')
    assert table['first'] == {'count': 3, 'weight': 0.5}
    assert table['second'] == {'count': 13, 'weight': 1.5}


chain = anlnext.AnalysisChain()
chain.console = False
chain.num_parallels = 2
chain.chain(MapExample, 'data')
values = [1, 2]
chain.set_parameter('vector_i', values)
values.append(3)
settings = {'vector_str': ['before'], 'map_str_str': {'tag': 'before'}}
chain.with_parameters(settings)
settings['vector_str'].append('extra')
settings['map_str_str']['tag'] = 'after'
row = {'count': 7}
chain.push_to_vector('rows', row)
row['count'] = 11
chain.push_to_vector('rows', row)
row['count'] = 99
record = {'count': 3, 'weight': 0.5}
chain.insert_to_map('table', 'first', record)
record['count'] = 13
record['weight'] = 1.5
chain.insert_to_map('table', 'second', record)
record['count'] = 99
late = {'count': 17}
chain.with_setter(lambda module: anlnext.push_to_vector(module, 'rows', late))
late['count'] = 23
assert chain.define() == anlnext.ANLStatus_ok
chain.load_all_parameters()
verify_values(chain.get_module('data'))


def verify_copies(c):
    for index in range(2):
        verify_values(c.get_parallel_module(index, 'data'))


chain.modify(verify_copies)
chain.run(0)

chain = anlnext.AnalysisChain()
chain.chain(SubmoduleExample, 'worker').with_submodule('algorithm', 'containers')
settings = {'algorithm_': {'samples': [3, 5], 'weights': {'left': 0.5}}}
chain.set_parameters(settings)
settings['algorithm_']['samples'].append(7)
settings['algorithm_']['weights']['left'] = 9.0
assert chain.define() == anlnext.ANLStatus_ok
chain.load_all_parameters()
algorithm = chain.get_module('worker').get_submodule('algorithm')
assert list(value(algorithm, 'samples')) == [3, 5]
assert value(algorithm, 'weights') == {'left': 0.5}
