#!/usr/bin/env ruby

require 'ANLLib' # ANL Next library
require 'myPackageMT'# Ruby extension library using ANL Next

# Define your own application class derived from ANL::ANLApp.
class MyApp < ANL::ANLApp
  # Define an analysis chain in setup() method.
  def setup()
    add_namespace MyPackageMT

    chain :MyMTModule
    with_parameters(MyParameter1: 10,
                    MyParameter2: 20.5,
                    MyParameter3: "Hello",
                    MyVector1: [1, 2, 3, 4, 5],
                    MyVector2: [1.3, 4.0, 11.2, 3.2],
                    MyVector3: ["Hakuba", "Niseko", "Appi"])

    # If you need to add the same type of module, you should set another name
    # via the second argument.
    chain :MyMTModule, :MyMTModule2
    with_parameters(MyParameter2: 102.1,
                    MyVector3: ["Jupiter", "Venus", "Mars", "Saturn"])
  end
end

a = MyApp.new
a.num_parallels = 4
a.run(1000000, 100000)
