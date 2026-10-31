import anlnext
from submodule_python import DefinitionStatusExample


def make_chain(status, parallels=1):
    chain = anlnext.AnalysisChain()
    chain.console = False
    chain.num_parallels = parallels
    chain.chain(lambda: DefinitionStatusExample(status), 'status')
    calls = []
    chain.with_setter(lambda module: calls.append('parameters'))
    chain.modify(lambda c: calls.append('modify'))
    return chain, calls


for status in [anlnext.ANLStatus_critical_error_to_terminate,
               anlnext.ANLStatus_critical_error_to_finalize, anlnext.ANLStatus_quit]:
    direct, _ = make_chain(status)
    assert direct.define() == status
    for parallels in [1, 2]:
        chain, calls = make_chain(status, parallels)
        chain.run(1)
        module = chain.get_module('status')
        assert calls == []
        assert module.pre_initialize_calls == 0
        assert module.initialize_calls == 0
        assert module.analyze_calls == 0
        assert module.finalize_calls == 0

chain, calls = make_chain(anlnext.ANLStatus_ok)
chain.run(1)
module = chain.get_module('status')
assert calls == ['parameters', 'modify']
assert module.pre_initialize_calls == 1
assert module.initialize_calls == 1
assert module.analyze_calls == 1
assert module.finalize_calls == 1
