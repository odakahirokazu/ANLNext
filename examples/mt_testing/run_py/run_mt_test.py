#!/usr/bin/env python3

import anlnext # ANL Next library
import myPackageMTPy as myp # Python extension library using ANL Next

### chain definition
def setup(module):
    module.chain(myp.MyMTModule)
    module.with_parameters({
        "quit_index": -1,
        "quit_all": True
    })

    module.chain(myp.MyMTModule, "MyMTModule2")
    module.with_parameters({
        "quit_index": 77777,
        "quit_all": True
    })

### run analysis chain
a = anlnext.AnalysisChain()
a.num_parallels = 4
setup(a)

def modify_param(module):
    module.get_parallel_module(2, "MyMTModule2").set_parameter("quit_index", 500)

a.modify(modify_param)

a.run(1000000)
