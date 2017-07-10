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

a = MyApp.new
a.setup
a.define
a.make_doc(output: "doc_detail.xml",
           namespace: "MyPackage")
