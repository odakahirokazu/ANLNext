#!/usr/bin/env ruby

require 'ANLLib'
require 'myPackage'

class MyApp < ANL::ANLApp
  def setup()
    chain MyPackage::MyModule
    chain MyPackage::MyMapModule
    chain MyPackage::MyVectorModule
  end
end

output = "run_myapp.rb"
package = "myPackage"
namespace = "MyPackage"

a = MyApp.new
a.setup
a.define
a.make_script(output: output,
              package: package,
              namespace: namespace)

puts ""
puts "A run script '#{output}' is generated."
puts "Edit #{output} and then run it."
