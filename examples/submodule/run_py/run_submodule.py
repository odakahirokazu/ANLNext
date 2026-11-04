import sys

import anlnext
from submoduleExamplePy import AlgorithmModule

chain = anlnext.AnalysisChain()
chain.console = False
chain.num_parallels = int(sys.argv[1]) if len(sys.argv) > 1 else 3

chain.chain(AlgorithmModule, 'multiply')
chain.with_submodule('algorithm', 'multiply')
chain.with_parameters({'input': 4.0, 'algorithm.factor': 7.0})

chain.chain(AlgorithmModule, 'offset')
chain.with_submodule('algorithm', 'offset')
chain.with_parameters({'input': 4.0, 'algorithm.offset': 3.0})

chain.run(10)

for module_id, name, value, total in [('multiply', 'factor', 7.0, 280.0), ('offset', 'offset', 3.0, 70.0)]:
    module = chain.get_module(module_id)
    algorithm = module.get_submodule('algorithm')
    print(f'{module_id}: {algorithm.submodule_name()}')
    algorithm.print_parameters()
    print(f'{name} = {anlnext.get_value(algorithm.get_parameter(name))}')
    print(f'total = {anlnext.get_value(module.get_parameter("total"))}')
    assert anlnext.get_value(module.get_parameter('total')) == total
    for index in range(chain.num_parallels):
        copy = chain.get_parallel_module(index, module_id).get_submodule('algorithm')
        assert anlnext.get_value(copy.get_parameter(name)) == value
