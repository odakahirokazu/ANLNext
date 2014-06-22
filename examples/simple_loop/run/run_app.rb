#!/usr/bin/env ruby
require 'ANLLib'
require 'myPackage'

n_loop = 100000
display_frequency = 1000

class TestApp < ANL::ANLApp
  def setup()
    add_namespace MyPackage

    chain :MyModule
    with_parameters("MyParameter1" => 1,
                    "MyParameter2" => 2,
                    "MyParameter3" => 3)

  end
end

anl = TestApp.new
anl.run(n_loop, display_frequency)
