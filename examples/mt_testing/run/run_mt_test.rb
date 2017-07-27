#!/usr/bin/env ruby

require 'anlnext' # ANL Next library
require 'myPackageMT'# Ruby extension library using ANL Next

# Define your own application class derived from ANL::ANLApp.
class MyApp < ANL::ANLApp
  # Define an analysis chain in setup() method.
  def setup()
    add_namespace MyPackageMT

    chain :MyMTModule
    with_parameters(quit_index: -1,
                    quit_all: true)

    # If you need to add the same type of module, you should set another name
    # via the second argument.
    chain :MyMTModule, :MyMTModule2
    with_parameters(quit_index: 77777,
                    quit_all: true)

  end
end

a = MyApp.new
a.num_parallels = 4

a.modify do |anl|
  anl.modify_parameters 2, :MyMTModule2, {
    quit_index: 500
  }
end

a.run(1000000)
