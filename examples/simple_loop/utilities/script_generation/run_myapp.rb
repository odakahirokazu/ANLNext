#!/usr/bin/env ruby
require 'anlnext'
require 'myPackage'

num_loop = 100000

class MyApp < ANL::ANLApp
  def setup()
    add_namespace MyPackage

    chain :MyModule
    with_parameters("my_parameter1" => 1,
                    "my_parameter2" => 2.0,
                    "my_parameter3" => "test",
                    "my_vector1" => [],
                    "my_vector2" => [],
                    "my_vector3" => [""],
                    "my_parameter11" => 200,
                    "my_parameter12" => 20000,
                    "my_parameter13" => 5000000000,
                    "my_parameter21" => true)

    chain :MyMapModule
    with_parameters()
    insert_to_map "my_map", "", {
      "ID" => 0,
      "type" => "pixel",
      "x" => 0.0,
      "y" => 0.0,
    }

    chain :MyVectorModule
    with_parameters()
    push_to_vector "my_vector", {
      "ID" => 0,
      "type" => "pixel",
      "x" => 0.0,
      "y" => 0.0,
    }

  end
end

anl = MyApp.new
anl.run(num_loop)
