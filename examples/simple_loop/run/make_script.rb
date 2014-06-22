#!/usr/bin/env ruby

require 'ANLLib'
require 'myPackage'

class MyApp < ANL::ANLApp
  def setup()
    chain MyPackage::MyModule
  end
end

a = MyApp.new
a.setup
a.startup
a.make_script("run_app.rb", "myPackage", "MyPackage", "TestApp")
