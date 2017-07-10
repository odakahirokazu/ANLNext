# histogram_mt

This is one of the simplest example of ANL application.
This makes simple histograms and save them into a ROOT file.

**CreateRootFile** creates a ROOT file for data recording.

**GenerateEvents** randomly generates a value of energy. This module has
parameters of the simulated detectors.

**FillHistogram** defines histograms and fill the energies generated in the privious module. This defines parameters of these histograms:
- nbin: type `int`, number of bins
- energy_min: type `double`, lower bound of the histograms
- energy_max: type `double`, upper bound of the histograms

## Directory structure

- source: the source tree defines ANL modules. Here is the main cmake file (CMakeLists.txt).
    * source/include: C++ header files (*.hh) declare the modules.
    * source/src: C++ source files (*.cc) define the modules.
    * source/rubyext: SWIG interface file to build a Ruby extension library.
- run: this directory a Ruby script (`run_test_histogram.rb`) that defines the ANL application. You can directly execute this script.

## How to build

    # pwd ===> /path/to/ANLNext/examples/histogram_mt
    mkdir build
    cd build
    cmake ../source -DCMAKE_INSTALL_PREFIX=/path/to/install

By default, the install destination is your home directory.

    make
    make install

## How to run

    # pwd ===> /path/to/ANLNext/examples/histogram_mt
    cd run
    ./run_test_histogram.rb

After the run, a ROOT file containing histograms should be generated. Check it!
