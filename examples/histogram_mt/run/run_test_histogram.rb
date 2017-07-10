#!/usr/bin/env ruby

require 'ANLLib'
require 'testHistogramMT'

class MyApp < ANL::ANLApp
  def setup()
    chain TestHistogramMT::CreateRootFile
    with_parameters(filename_base: "output",
                    parallel: true,
                    save_parallel: false)

    chain TestHistogramMT::GenerateEvents
    with_parameters(energy: 120.0,
                    sigma: 4.0,
                    num_detectors: 1000,
                    efficiency: 0.03,
                    random_seed: 500)

    chain TestHistogramMT::FillHistogram
    with_parameters(nbin: 128,
                    energy_min: 80.0,
                    energy_max: 150.0)
  end
end

a = MyApp.new
a.num_parallels = 4
a.run(1000000, 10000)
