#!/usr/bin/env ruby

require 'ANLLib'
require 'myPackage'

# Define your own application class derived from ANL::ANLApp.
class MyApp < ANL::ANLApp
  # Define an analysis chain in setup() method.
  def setup()
    chain MyPackage::MyModule
    chain MyPackage::MyVectorModule
    chain MyPackage::MyMapModule
  end
end

a = MyApp.new
a.run_interactive
