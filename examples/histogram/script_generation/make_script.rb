#!/usr/bin/env ruby

require 'ANLLib'
require 'testHistogram'

class MyApp < ANL::ANLApp
  def setup()
    chain TestHistogram::SaveData
    chain TestHistogram::GenerateEvents
    chain TestHistogram::FillHistogram
  end
end

a = MyApp.new
a.setup
a.define
a.make_script(output: "run_app.rb",
              package: "testHistogram",
              namespace: "TestHistogram")
