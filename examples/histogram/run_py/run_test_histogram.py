#!/usr/bin/env python3

import anlnext # ANL Next library
import testHistogramPy as th # Python extension library using ANL Next

### chain definition
def setup(chain):
    chain.append(th.SaveData)
    chain.with_parameters({
        "filename": "output.root"
    })

    chain.append(th.GenerateEvents)
    chain.with_parameters({
        "energy": 120.0,
        "detector1_sigma": 1.0,
        "detector2_sigma": 5.0
    })

    chain.append(th.FillHistogram)
    chain.with_parameters({
        "nbin": 128,
        "energy_min": 80.0,
        "energy_max": 150.0
    })

### run analysis chain
a = anlnext.AnalysisChain()
setup(a)
a.run(1000000)
