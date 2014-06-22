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
a.startup
a.make_script("run_app.rb", "testHistogram", "TestHistogram", "TestApp")
