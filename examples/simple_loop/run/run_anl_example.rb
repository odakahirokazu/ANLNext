#!/usr/bin/env ruby

require 'ANLLib' # ANL Next library
require 'myPackage'# Ruby extension library using ANL Next

# Define your own application class derived from ANL::ANLApp.
class MyApp < ANL::ANLApp
  # Define an analysis chain in setup() method.
  def setup()
    chain MyPackage::MyModule
    with_parameters(MyParameter1: 10,
                    MyParameter2: 20,
                    MyParameter3: 30)

    # If you need to add the same type of module, you should set another name
    # via the second argument.
    chain MyPackage::MyModule, :MyModule2
    with_parameters(MyParameter2: 21)
  end
end

a = MyApp.new
a.run(1000000, 100000)
