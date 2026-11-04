import anlnext
from submodule_python import SubmoduleExample

chain = anlnext.AnalysisChain()
chain.chain(SubmoduleExample, 'first').with_submodule('algorithm', 'multiply')
settings = {'algorithm_': {'factor': 7}}
chain.with_parameters(settings)
settings['algorithm_']['factor'] = 99
chain.chain(SubmoduleExample, 'second').with_submodule('algorithm', 'containers')
chain.with_parameters({'algorithm.factor': 11})
assert chain.define() == anlnext.ANLStatus_ok
chain.load_all_parameters()
first = chain.get_module('first').get_submodule('algorithm')
second = chain.get_module('second').get_submodule('algorithm')
assert anlnext.get_value(first.get_parameter('factor')) == 7
assert anlnext.get_value(second.get_parameter('factor')) == 11
chain.with_parameters({'input': 4, 'algorithm.factor': 12,
                       'algorithm_': {'factor': 13, 'samples': [2, 3], 'weights': {'left_': 0.5, 'a.b': 1.5}}})
assert anlnext.get_value(second.get_parameter('factor')) == 11
chain.load_all_parameters()
assert anlnext.get_value(second.get_parameter('factor')) == 13
assert list(anlnext.get_value(second.get_parameter('samples'))) == [2, 3]
assert anlnext.get_value(second.get_parameter('weights')) == {'left_': 0.5, 'a.b': 1.5}
assert anlnext.get_value(chain.get_module('second').get_parameter('input')) == 4
assert anlnext.get_value(first.get_parameter('factor')) == 7
chain.with_parameters({'algorithm_': {'factor': 14}, 'algorithm.factor': 15})
chain.load_all_parameters()
assert anlnext.get_value(second.get_parameter('factor')) == 15
chain.with_parameters({'algorithm_': {}})
chain.load_all_parameters()
assert anlnext.get_value(second.get_parameter('factor')) == 15

for submodule, parameters, error in [
    ('missing', {'factor': 1}, 'SubModuleUndefinedError'),
    ('optional', {'factor': 1}, 'SubModuleNotSetError'),
    ('algorithm', {'missing': 1}, 'ParameterNotFound'),
]:
    for settings in [
        {f'{submodule}.{name}': value for name, value in parameters.items()},
        {f'{submodule}_': parameters},
    ]:
        invalid = anlnext.AnalysisChain()
        invalid.chain(SubmoduleExample).with_submodule('algorithm', 'multiply')
        invalid.with_parameters(settings)
        assert invalid.define() == anlnext.ANLStatus_ok
        try:
            invalid.load_all_parameters()
        except RuntimeError as exception:
            assert error in str(exception)
        else:
            raise AssertionError(f'Expected {error}')

for settings in [{'algorithm_': 1}, {'algorithm_': [1]}, {'_': {}}]:
    try:
        chain.with_parameters(settings)
    except ValueError:
        pass
    else:
        raise AssertionError('Expected invalid submodule parameter group')

for name in ['.factor', 'algorithm.', 'algorithm.factor.extra']:
    invalid = anlnext.AnalysisChain()
    invalid.chain(SubmoduleExample).with_submodule('algorithm', 'multiply')
    invalid.with_parameters({name: 1})
    assert invalid.define() == anlnext.ANLStatus_ok
    try:
        invalid.load_all_parameters()
    except ValueError:
        pass
    else:
        raise AssertionError('Expected invalid parameter path')
