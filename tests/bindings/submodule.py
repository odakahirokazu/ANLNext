import json
import tempfile
from pathlib import Path

import anlnext
from submodule_python import SubmoduleExample


def expect_error(action, text):
    try:
        action()
    except RuntimeError as error:
        assert text in str(error), str(error)
    else:
        raise AssertionError("expected RuntimeError")


def configure(module):
    assert module.is_submodule_defined("algorithm")
    assert not module.is_submodule_set("algorithm")
    expect_error(lambda: module.get_submodule("algorithm"), "SubModuleNotSetError")
    expect_error(lambda: module.get_submodule("missing"), "SubModuleUndefinedError")
    expect_error(lambda: module.set_submodule_by_key("algorithm", "unknown"), "Unknown algorithm")
    module.set_submodule_by_key("algorithm", "multiply")
    module.get_submodule("algorithm").set_parameter_integer("factor", 99)
    module.set_submodule_by_key("algorithm", "multiply")
    algorithm = module.get_submodule("algorithm")
    assert anlnext.get_value(algorithm.get_parameter("factor")) == 2


def modify(chain):
    for index in range(chain.num_parallels):
        module = chain.get_parallel_module(index, "example")
        algorithm = module.get_submodule("algorithm")
        assert anlnext.get_value(algorithm.get_parameter("factor")) == 7
        assert not module.is_submodule_set("optional")
        expect_error(lambda: module.set_submodule_by_key("algorithm", "multiply"),
                     "SubModuleConfigurationError")
        algorithm.set_parameter_integer("factor", 7 + index)


chain = anlnext.AnalysisChain()
chain.console = False
chain.num_parallels = 3
chain.chain(SubmoduleExample, "example").with_setter(configure).with_parameters({"input": 4})
chain.with_parameters({'algorithm.factor': 7})
chain.modify(modify)
chain.run(6)
for index in range(chain.num_parallels):
    module = chain.get_parallel_module(index, "example")
    assert anlnext.get_value(module.get_parameter("result")) == 4 * (7 + index)

with tempfile.TemporaryDirectory() as directory:
    filename = Path(directory) / "parameters.json"
    chain.anl.parameters_to_json(str(filename))
    application = json.loads(filename.read_text())["application"]
    for index in range(chain.num_parallels):
        modules = application["module_list" if index == 0 else f"chain{index}"]
        entries = {entry["registered_name"]: entry for entry in modules[0]["submodule_list"]}
        algorithm = entries["algorithm"]
        assert algorithm["name"] == "ExampleAlgorithm"
        assert algorithm["version"] == "1.0"
        assert algorithm["state"] == "set"
        assert int(algorithm["parameter_list"][0]["value"]) == 7 + index
        assert entries["optional"] == {"registered_name": "optional", "state": "unset"}
