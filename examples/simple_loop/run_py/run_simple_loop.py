#!/usr/bin/env python3

import anlnext # ANL Next library
import myPackagePy as myp # Python extension library using ANL Next

### chain definition
def setup(module):
    module.chain(myp.MyModule).with_parameters({
        "my_parameter1": 10,
        "my_parameter2": 20.5,
        "my_parameter3": "Hello",
        "my_vector1": [1, 2, 3, 4, 5],
        "my_vector2": [1.3, 4.0, 11.2, 3.2],
        "my_vector3": ["Hakuba", "Niseko", "Appi"]
    })

    module.chain(myp.MyVectorModule)
    module.push_to_vector("my_vector", {"ID": 1, "type": "strip", "x": 0.0, "y": 0.0})
    module.push_to_vector("my_vector", {"ID": 3, "type": "pixel", "x": -0.2, "y": 4.0})
    module.push_to_vector("my_vector", {"ID": 4, "type": "pixel", "x": -0.2, "y": 8.0})

    module.chain(myp.MyMapModule)
    module.insert_to_map("my_map", "Si1",   {"ID": 1, "type": "strip", "x": 0.0, "y": 0.0})
    module.insert_to_map("my_map", "Si2",   {"ID": 2, "type": "strip", "x": 0.0, "y": 2.0})
    module.insert_to_map("my_map", "CdTe1", {"ID": 3, "type": "pixel", "x": -0.2, "y": 4.0})
    module.insert_to_map("my_map", "CdTe2", {"ID": 4, "type": "pixel", "x": -0.2, "y": 8.0})

    module.chain(myp.MyModule, "MyModule2").with_parameters({
        "my_parameter2": 120.0,
        "my_vector3": ["Jupiter", "Venus", "Mars", "Saturn"]
    })

    module.chain(myp.MyModule, "MyModule3").with_parameters({
        "my_parameter11": -301,
        "my_parameter12": 1000000000,
        "my_parameter13": 876543210
    })


### run analysis chain
a = anlnext.AnalysisChain()
setup(a)
a.run(10000)


### to peek a parameter
print(anlnext.get_value(a.get_module("MyModule").get_parameter("my_parameter11")))
print(anlnext.get_value(a.get_module("MyVectorModule").get_parameter("my_vector")))
print(anlnext.get_value(a.get_module("MyMapModule").get_parameter("my_map")))
