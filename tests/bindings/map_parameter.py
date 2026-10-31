import json
import tempfile
from pathlib import Path
import anlnext
from submodule_python import MapExample


def verify_record_integer_range():
    module = MapExample()
    manager = anlnext.ANLManager()
    manager.set_modules([module])
    manager.define_analysis()
    for name in ['table', 'rows']:
        parameter = module.get_parameter(name)

        def insert(value, key):
            if name == 'table':
                module.insert_to_map(name, key, {'count': value})
            else:
                module.push_to_vector(name, {'count': value})

        for index, value in enumerate([-2147483648, -1, 0, 2147483647]):
            key = str(index)
            insert(value, key)
            stored = anlnext.get_value(parameter)
            row = stored[key] if name == 'table' else stored[-1]
            assert row['count'] == value

        expected = anlnext.get_value(parameter)
        expected_size = parameter.size_of_container()
        for bad in [2147483648, -2147483649, (1 << 32) + 7, 1 << 100, -(1 << 100)]:
            try:
                insert(bad, 'invalid')
            except RuntimeError as error:
                assert 'ParameterError' in str(error) and 'count' in str(error)
            else:
                raise AssertionError(f'accepted out-of-range record integer: {name}: {bad}')
            assert parameter.size_of_container() == expected_size
            assert anlnext.get_value(parameter) == expected


verify_record_integer_range()

values = {
    'map_i_i': {-1: 3, 2147483647: -2147483648},
    'map_i_d': {-1: 0.5, 2: 1.5},
    'map_i_str': {-1: '', 2: 'a b"c\\d'},
    'map_str_i': {'': 3, 'a.b': 5},
    'map_str_d': {'a b': 0.5, 'a.b': 1.5},
    'map_str_str': {'': '', 'a b': 'q"\\r'},
}
chain = anlnext.AnalysisChain()
chain.console = False
chain.num_parallels = 2
chain.chain(MapExample, 'maps').with_parameters(values)
chain.with_setter(lambda m: m.insert_to_map('table', 'row', {'count': 3, 'weight': 0.5}))
chain.push_to_vector('rows', {'count': 4})

def modify(c):
    original = c.get_parallel_module(0, 'maps')
    copied = c.get_parallel_module(1, 'maps')
    assert original.get_parameter('table').type_name() == 'record_map'
    assert original.get_parameter('rows').type_name() == 'record_vector'
    assert anlnext.get_value(original.get_parameter('table')) == {'row': {'count': 3, 'weight': 0.5}}
    assert anlnext.get_value(original.get_parameter('rows')) == [{'count': 4}]
    for name, expected in values.items():
        assert anlnext.get_value(original.get_parameter(name)) == expected
        assert anlnext.get_value(copied.get_parameter(name)) == expected
        anlnext.set_map_parameter(copied, name, {})
        assert anlnext.get_value(copied.get_parameter(name)) == {}
        assert anlnext.get_value(original.get_parameter(name)) == expected
    for name, bad in [('map_i_i', {'1': 3}), ('map_i_i', {1.5: 3}),
                      ('map_i_i', {2147483648: 1}), ('map_i_i', {1: 2147483648}),
                      ('map_i_i', {1: 1.5}), ('map_str_i', {1: 2}),
                      ('map_i_d', {1: 'bad'}), ('map_i_str', {1: 2})]:
        try:
            anlnext.set_map_parameter(original, name, bad)
        except (TypeError, OverflowError, ValueError):
            pass
        else:
            raise AssertionError(f'accepted invalid map: {name}: {bad}')
        assert anlnext.get_value(original.get_parameter(name)) == values[name]
    try:
        original.get_parameter('map_i_i').get_value_map_str_str({})
    except RuntimeError:
        pass
    else:
        raise AssertionError('incorrect getter type accepted')

chain.modify(modify)
chain.run(0)
with tempfile.TemporaryDirectory() as directory:
    filename = Path(directory) / 'maps.json'
    chain.anl.parameters_to_json(str(filename))
    parameters = json.loads(filename.read_text())['application']['module_list'][0]['parameter_list']
    for param in parameters:
        if param['name'] not in values:
            continue
        assert len(param['value']) == len(values[param['name']])
        if param['name'].endswith('_d'):
            assert float(param['unit']) == 10.0
