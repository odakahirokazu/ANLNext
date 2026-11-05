import json
import tempfile
from pathlib import Path
import anlnext
from submodule_python import UnsignedIntegerExample

maximum = UnsignedIntegerExample.maximum()
signed_maximum = UnsignedIntegerExample.signed_maximum()
assert maximum >= (1 << 64) - 1


def expect_error(action):
    try:
        action()
    except (RuntimeError, OverflowError, TypeError, ValueError):
        return
    raise AssertionError('expected error')


chain = anlnext.AnalysisChain()
chain.console = False
chain.num_parallels = 2
chain.chain(UnsignedIntegerExample, 'unsigned').with_parameters({
    'address': maximum, 'count': 7, 'signed_value': -23,
})


def modify(c):
    original = c.get_parallel_module(0, 'unsigned')
    copied = c.get_parallel_module(1, 'unsigned')
    param = original.get_parameter('address')
    assert param.type_name() == 'unsigned_integer'
    assert anlnext.get_value(param) == maximum
    assert anlnext.get_value(original.get_parameter('signed_value')) == -23
    for value in [0, signed_maximum, signed_maximum + 1, maximum]:
        copied.set_parameter_unsigned_integer('address', value)
        assert copied.get_parameter('address').get_value_unsigned_integer() == value
        assert anlnext.get_value(param) == maximum
    expect_error(lambda: param.get_value_integer())
    expect_error(lambda: param.get_value(0))
    for bad in [-1, maximum + 1, 1.5, '123']:
        expect_error(lambda: original.set_parameter_unsigned_integer('address', bad))
        assert anlnext.get_value(param) == maximum
    expect_error(lambda: original.set_parameter_integer('address', -1))
    expect_error(lambda: original.set_parameter_unsigned_integer('count', maximum))
    expect_error(lambda: original.get_parameter('count').get_value_unsigned_integer())


chain.modify(modify)
chain.run(0)
for bad in [-1, maximum + 1]:
    invalid = anlnext.AnalysisChain()
    invalid.chain(UnsignedIntegerExample, 'unsigned').with_parameters({'address': maximum})
    assert invalid.define() == anlnext.ANLStatus_ok
    invalid.load_all_parameters()
    invalid.with_parameters({'address': bad})
    expect_error(invalid.load_all_parameters)
    assert anlnext.get_value(invalid.get_module('unsigned').get_parameter('address')) == maximum

with tempfile.TemporaryDirectory() as directory:
    filename = Path(directory) / 'unsigned.json'
    chain.anl_manager.parameters_to_json(str(filename))
    application = json.loads(filename.read_text())['application']
    for key in ['module_list', 'chain1']:
        parameters = {p['name']: p for p in application[key][0]['parameter_list']}
        assert parameters['address']['type'] == 'unsigned_integer'
        assert parameters['address']['value'] == str(maximum)
