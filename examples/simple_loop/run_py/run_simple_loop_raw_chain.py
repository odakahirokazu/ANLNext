#!/usr/bin/env python3

import anlnext # ANL Next library
import myPackagePy as myp # Python extension library using ANL Next

am = anlnext.ANLManager()

m1 = myp.MyModule()
m2 = myp.MyVectorModule()
m3 = myp.MyMapModule()
m4 = myp.MyModule()
m4.set_module_id("MyModule2")
m5 = myp.MyModule()
m5.set_module_id("MyModule3")

am.set_modules([m1, m2, m3, m4, m5])

am.Define()

m1.set_parameter("my_parameter1", 10)
m1.set_parameter("my_parameter2", 20.5)
m1.set_parameter("my_parameter3", "Hello")
m1.set_parameter_vector_i("my_vector1", [1, 2, 3, 4, 5])
m1.set_parameter_vector_d("my_vector2", [1.3, 4.0, 11.2, 3.2])
m1.set_parameter_vector_str("my_vector3", ["Hakuba", "Niseko", "Appi"])

m2.push_to_vector("my_vector", {"ID": 1, "type": "strip", "x": 0.0, "y": 0.0})
m2.push_to_vector("my_vector", {"ID": 3, "type": "pixel", "x": -0.2, "y": 4.0})
m2.push_to_vector("my_vector", {"ID": 4, "type": "pixel", "x": -0.2, "y": 8.0})

m3.insert_to_map("my_map", "Si1",   {"ID": 1, "type": "strip", "x": 0.0, "y": 0.0})
m3.insert_to_map("my_map", "Si2",   {"ID": 2, "type": "strip", "x": 0.0, "y": 2.0})
m3.insert_to_map("my_map", "CdTe1", {"ID": 3, "type": "pixel", "x": -0.2, "y": 4.0})
m3.insert_to_map("my_map", "CdTe2", {"ID": 4, "type": "pixel", "x": -0.2, "y": 8.0})

m4.set_parameter("my_parameter2", 120.0)
m4.set_parameter_vector_str("my_vector3", ["Jupiter", "Venus", "Mars", "Saturn"])

m5.set_parameter("my_parameter11", -301)
m5.set_parameter("my_parameter12", 1000000000)
m5.set_parameter("my_parameter13", 876543210)

am.PreInitialize()
am.Initialize()
am.Analyze(10000)
am.Finalize()

### to peek a parameter
print(anlnext.get_value(m1.get_parameter("my_parameter11")))
print(anlnext.get_value(m2.get_parameter("my_vector")))
print(anlnext.get_value(m3.get_parameter("my_map")))
