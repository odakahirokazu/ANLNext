import anlnext
from submodule_python import MapExample


def expect_error(action):
    try:
        action()
    except (TypeError, RuntimeError, OverflowError, ValueError):
        return
    raise AssertionError('expected an invalid vector setting to fail')


chain = anlnext.AnalysisChain()
chain.chain(MapExample, 'vectors')
empty = {'vector_i': [], 'vector_d': [], 'vector_str': []}
chain.with_parameters(empty)
chain.define()
module = chain.get_module('vectors')
defaults = {'vector_i': [1, 2, 3], 'vector_d': [1.0, 2.0], 'vector_str': ['default']}
for name, expected in defaults.items():
    assert list(anlnext.get_value(module.get_parameter(name))) == expected
chain.load_all_parameters()
for name in empty:
    assert list(anlnext.get_value(module.get_parameter(name))) == []

values = {'vector_i': [7, -3], 'vector_d': [1, 2.5], 'vector_str': ['a b', '雪"\\\0']}
chain.with_parameters(values)
chain.load_all_parameters()
for name, expected in values.items():
    assert list(anlnext.get_value(module.get_parameter(name))) == expected
    anlnext.set_vector_parameter(module, name, [])
    assert list(anlnext.get_value(module.get_parameter(name))) == []
    anlnext.set_vector_parameter(module, name, expected)

for name, bad in [('vector_i', [1.5]), ('vector_i', ['7']),
                  ('vector_i', [0, 2147483648]), ('vector_i', [None]),
                  ('vector_d', ['bad']), ('vector_str', [7])]:
    expect_error(lambda: anlnext.set_vector_parameter(module, name, bad))
    assert list(anlnext.get_value(module.get_parameter(name))) == values[name]

expect_error(lambda: anlnext.set_vector_parameter(module, 'map_i_i', []))
expect_error(lambda: anlnext.set_vector_parameter(module, 'missing', []))
chain.with_parameters({'missing': []})
expect_error(chain.load_all_parameters)
