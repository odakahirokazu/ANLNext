#!/usr/bin/env python3

import anlnext
import testHistogramMTPy as th

### chain definition
def setup(module):
    module.chain(th.CreateRootFile)
    module.with_parameters({
        "filename_base": "output",
        "parallel": True,
        "save_parallel": False
    })

    module.chain(th.GenerateEvents)
    module.with_parameters({
        "energy": 120.0,
        "sigma": 4.0,
        "num_detectors": 1000,
        "efficiency": 0.03,
        "random_seed": 500
    })

    module.chain(th.FillHistogram)
    module.with_parameters({
        "nbin": 128,
        "energy_min": 80.0,
        "energy_max": 150.0
    })


### run analysis chain
a = anlnext.AnalysisChain()
a.num_parallels = 4
setup(a)

def modify_param(module):
    module.get_parallel_module(0, "GenerateEvents").set_parameter("energy", 90.0)
    module.get_parallel_module(0, "GenerateEvents").set_parameter("sigma", 6.0)

a.modify(modify_param)

a.run(1000000)
