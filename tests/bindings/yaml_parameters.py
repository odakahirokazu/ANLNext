from pathlib import Path
import tempfile

import yaml
import anlnext
from submodule_python import SubmoduleExample, MapExample, UnsignedIntegerExample


def make_chain():
    chain = anlnext.AnalysisChain()
    chain.console = False
    chain.num_parallels = 3
    for module_id in ['first', 'second']:
        chain.chain(SubmoduleExample, module_id).with_submodule('algorithm', 'containers')
    chain.chain(MapExample, 'maps')
    chain.chain(UnsignedIntegerExample, 'unsigned')
    return chain


filename = Path(__file__).with_suffix('.yaml')
chain = make_chain()
current = chain.current_module
chain.with_parameters({'count': 1})
assert chain.load_parameters_from_yaml(filename) is chain
assert chain.current_module is current
chain.with_parameters({'count': 5})
with filename.open(encoding='utf-8') as stream:
    expected_maps = yaml.safe_load(stream)['maps']


def verify(c):
    for index in range(3):
        for module_id, factor in [('first', 7), ('second', 11)]:
            mod = c.get_parallel_module(index, module_id)
            algorithm = mod.get_submodule('algorithm')
            assert anlnext.get_value(algorithm.get_parameter('factor')) == factor
            assert list(anlnext.get_value(algorithm.get_parameter('samples'))) == [2, 3]
            assert anlnext.get_value(algorithm.get_parameter('weights')) == {'left_': 0.5, 'a.b': 1.5}
            assert anlnext.get_value(algorithm.get_parameter('enabled')) is (module_id == 'first')
            assert anlnext.get_value(algorithm.get_parameter('label')) == 'on'
        maps = c.get_parallel_module(index, 'maps')
        for name, expected in expected_maps.items():
            value = anlnext.get_value(maps.get_parameter(name))
            if name.startswith('vector_'):
                value = list(value)
            assert value == expected
        unsigned = c.get_parallel_module(index, 'unsigned')
        assert anlnext.get_value(unsigned.get_parameter('address')) == UnsignedIntegerExample.maximum()
        assert anlnext.get_value(unsigned.get_parameter('signed_value')) == -9223372036854775808
        assert anlnext.get_value(unsigned.get_parameter('count')) == 5


chain.modify(verify)
chain.run(0)

with tempfile.TemporaryDirectory() as directory:
    file = Path(directory) / 'parameters.yaml'
    for text in ['', '[]', 'null', 'unsigned:', '1: {}', 'unsigned: {1: 7}',
                 'unsigned: {count: 9}\nmissing: {}', 'unsigned: {', '!ruby/object:Object {}',
                 'unsigned: {}\n---\nunsigned: {}', 'first: {algorithm_: 1}']:
        invalid = make_chain()
        selected = invalid.current_module
        file.write_text(text, encoding='utf-8')
        try:
            invalid.load_parameters_from_yaml(file)
        except (ValueError, yaml.YAMLError):
            assert invalid.current_module is selected
        else:
            raise AssertionError(f'Accepted invalid YAML: {text}')

    invalid = make_chain()
    file.write_text('first: {algorithm_: {missing: 1}}', encoding='utf-8')
    invalid.load_parameters_from_yaml(file)
    assert invalid.define() == anlnext.ANLStatus_ok
    try:
        invalid.load_all_parameters()
    except RuntimeError as error:
        assert 'ParameterNotFound' in str(error)
    else:
        raise AssertionError('Accepted an unknown parameter')

    chain = make_chain()
    assert chain.define() == anlnext.ANLStatus_ok
    chain.load_all_parameters()
    file.write_text('unsigned: {count: 17}', encoding='utf-8')
    chain.load_parameters_from_yaml(file)
    assert anlnext.get_value(chain.get_module('unsigned').get_parameter('count')) == 0
    chain.load_all_parameters()
    assert anlnext.get_value(chain.get_module('unsigned').get_parameter('count')) == 17
