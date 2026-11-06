#!/usr/bin/env python3

import anlnext # ANL Next library
import myPackageMTPy as myp # Python extension library using ANL Next

### chain definition
def setup(chain):
    chain.append(myp.MyMTModule)
    chain.with_parameters({
        "quit_index": -1,
        "quit_all": True
    })

    chain.append(myp.MyMTModule, "MyMTModule2")
    chain.with_parameters({
        "quit_index": 77777,
        "quit_all": True
    })

### run analysis chain
chain = anlnext.AnalysisChain()
chain.num_parallels = 4
setup(chain)

def modify_param(chain):
    chain.get_parallel_module(2, "MyMTModule2").set_parameter("quit_index", 500)

chain.modify(modify_param)

chain.run(1000000)
