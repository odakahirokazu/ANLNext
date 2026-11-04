import gc
import json
import tempfile
from pathlib import Path
import anlnext
from submodule_python import MapExample

for name in ['table', 'rows']:
    module = MapExample()
    manager = anlnext.ANLManager()
    manager.set_modules([module])
    assert manager.define_analysis() == anlnext.ANLStatus_ok
    if name == 'table':
        anlnext.insert_to_map(module, name, 'stored.key', {'count': 3})
        key = 'stored.key'
    else:
        anlnext.push_to_vector(module, name, {'count': 3})
        key = 0
    parameter = module.get_parameter(name)
    module.expose_parameter(name)
    module.set_map_key('edited')
    module.set_value_element('count', 41)
    buffer = parameter.value_element_info(0)
    assert buffer.get_value(0) == 41
    assert 'count: 3' in parameter.value_string()
    assert buffer.get_value(0) == 41
    stored = anlnext.get_value(parameter)
    assert (stored[key] if name == 'table' else stored[0])['count'] == 3
    assert buffer.get_value(0) == 41
    snapshots = parameter.value_elements_from_container(key)
    element = snapshots[0]
    assert anlnext.get_value(element) == 3
    del snapshots
    gc.collect()
    assert anlnext.get_value(element) == 3
    assert buffer.get_value(0) == 41
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / 'parameters.json'
        manager.parameters_to_json(str(path))
        data = json.loads(path.read_text())
        assert data['application']['module_list'][0]['parameter_list']
    assert buffer.get_value(0) == 41
    module.insert_to_container()
    stored = anlnext.get_value(parameter)
    assert (stored['edited'] if name == 'table' else stored[-1])['count'] == 41
    assert buffer.get_value(0) == 0
    assert anlnext.get_value(element) == 3
    try:
        parameter.value_elements_from_container('missing' if name == 'table' else 99)
    except RuntimeError:
        pass
    else:
        raise AssertionError('accepted a missing record')
    assert buffer.get_value(0) == 0
