import anlnext
from anlnext import status
from submodule_python import DefinitionStatusExample


names = [
    'proceed', 'proceed_with_error', 'skip', 'skip_with_error',
    'quit', 'quit_with_error', 'quit_all', 'quit_all_with_error',
    'critical_error_to_finalize', 'critical_error_to_terminate',
    'critical_error_to_finalize_from_exception',
    'critical_error_to_terminate_from_exception',
]
for value, name in enumerate(names):
    code = getattr(anlnext, 'ANLStatus_' + name)
    assert code == value
    assert anlnext.status_to_string(code) == 'AS_' + name.upper()
    if not name.endswith('_from_exception'):
        assert getattr(status, 'AS_' + name.upper()) == code

for name, expected in [
    ('proceed_with_error', status.AS_PROCEED), ('skip_with_error', status.AS_PROCEED),
    ('quit_with_error', status.AS_QUIT), ('quit_all_with_error', status.AS_QUIT),
]:
    code = getattr(status, 'AS_' + name.upper())
    module = DefinitionStatusExample(code)
    manager = anlnext.ANLManager()
    manager.set_modules([module])
    assert manager.define_analysis() == expected
