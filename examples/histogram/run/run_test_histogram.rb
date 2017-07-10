#!/usr/bin/env ruby

require 'ANLLib'
require 'testHistogram'

class MyApp < ANL::ANLApp
  def setup()
    chain TestHistogram::SaveData
    with_parameters(filename: "output.root")

    chain TestHistogram::GenerateEvents
    with_parameters(energy: 120.0,
                    detector1_sigma: 1.0,
                    detector2_sigma: 5.0)

    chain TestHistogram::FillHistogram
    with_parameters(nbin: 128,
                    energy_min: 80.0,
                    energy_max: 150.0)
  end
end

a = MyApp.new
a.thread_mode = false
a.run(1000000)
