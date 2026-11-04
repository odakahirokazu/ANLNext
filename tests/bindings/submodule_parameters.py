import anlnext
from submodule_python import SubmoduleExample

chain = anlnext.AnalysisChain()
chain.chain(SubmoduleExample, 'first').with_submodule('algorithm', 'multiply')
settings = {'algorithm.factor': 7}
chain.with_parameters(settings)
settings['algorithm.factor'] = 99
chain.chain(SubmoduleExample, 'second').with_submodule('algorithm', 'multiply')
chain.with_parameters({'algorithm.factor': 11})
assert chain.define() == anlnext.ANLStatus_ok
chain.load_all_parameters()
first = chain.get_module('first').get_submodule('algorithm')
second = chain.get_module('second').get_submodule('algorithm')
assert anlnext.get_value(first.get_parameter('factor')) == 7
assert anlnext.get_value(second.get_parameter('factor')) == 11
chain.with_parameters({'input': 4, 'algorithm.factor': 13})
assert anlnext.get_value(second.get_parameter('factor')) == 11
chain.load_all_parameters()
assert anlnext.get_value(second.get_parameter('factor')) == 13
assert anlnext.get_value(chain.get_module('second').get_parameter('input')) == 4
assert anlnext.get_value(first.get_parameter('factor')) == 7

for submodule, parameters, error in [
    ('missing', {'factor': 1}, 'SubModuleUndefinedError'),
    ('optional', {'factor': 1}, 'SubModuleNotSetError'),
    ('algorithm', {'missing': 1}, 'ParameterNotFound'),
]:
    invalid = anlnext.AnalysisChain()
    invalid.chain(SubmoduleExample).with_submodule('algorithm', 'multiply')
    invalid.with_parameters({f'{submodule}.{name}': value for name, value in parameters.items()})
    assert invalid.define() == anlnext.ANLStatus_ok
    try:
        invalid.load_all_parameters()
    except RuntimeError as exception:
        assert error in str(exception)
    else:
        raise AssertionError(f'Expected {error}')

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
