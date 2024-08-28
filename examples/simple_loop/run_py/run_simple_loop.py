#!/usr/bin/env python3

from anlnext import anlpy # ANL Next library
import myPackagePy as myp # Python extension library using ANL Next

am = anlpy.ANLManager()

m1 = myp.MyModule()
m2 = myp.MyVectorModule()

am.set_modules([m1, m2])

am.Define()

m1.set_parameter("my_parameter1", 11)
m1.set_parameter("my_parameter2", 12.2)

m2.push_to_vector("my_vector", {"ID": 3, "type": "strip", "x": 0.0, "y": -0.2})

am.Initialize()
am.Analyze(20)
am.Finalize()

# class MyApp < ANL::ANLApp
#   # Define an analysis chain in setup() method.
#   def setup()
#     chain MyPackage::MyModule
#     with_parameters(my_parameter1: 10,
#                     my_parameter2: 20.5,
#                     my_parameter3: "Hello",
#                     my_vector1: [1, 2, 3, 4, 5],
#                     my_vector2: [1.3, 4.0, 11.2, 3.2],
#                     my_vector3: ["Hakuba", "Niseko", "Appi"])

#     chain MyPackage::MyVectorModule
#     with_parameters()
#     push_to_vector :my_vector, {ID: 1, type: "strip", x: 0.0, y: 0.0}
#     push_to_vector :my_vector, {ID: 3, type: "pixel", x: -0.2, y: 4.0}
#     push_to_vector :my_vector, {ID: 4, type: "pixel", x: -0.2, y: 6.0}

#     chain MyPackage::MyMapModule
#     with_parameters()
#     insert_to_map :my_map, "Si1",   {ID: 1, type: "strip", x: 0.0, y: 0.0}
#     insert_to_map :my_map, "Si2",   {ID: 2, type: "strip", x: 0.0, y: 2.0}
#     insert_to_map :my_map, "CdTe1", {ID: 3, type: "pixel", x: -0.2, y: 4.0}
#     insert_to_map :my_map, "CdTe2", {ID: 4, type: "pixel", x: -0.2, y: 6.0}

#     # If you need to add the same type of module, you should set another name
#     # via the second argument.
#     chain MyPackage::MyModule, :MyModule2
#     with_parameters(my_parameter2: 102.1,
#                     my_vector3: ["Jupiter", "Venus", "Mars", "Saturn"])

#     chain MyPackage::MyModule, :MyModule3
#     with_parameters(my_parameter11: -301,
#                     my_parameter12: 1000000000,
#                     my_parameter13: 9876543210)
#   end
# end

# a = MyApp.new
# a.write_parameters_to_json "parameters.json"
# a.run(1000000, display_period=100000)
