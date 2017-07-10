# histogram

This is one of the simplest example of ANL application.
This makes simple histograms and save them into a ROOT file.

**SaveData** creates a ROOT file for data recording. This module has a
parameter of output filename.
- filename: type `std::string`

**GenerateEvents** randomly generates a value of energy. This module has
parameters of the simulated detectors:
- energy: type `double`, the mean energy to be generated
- detector1_sigma: type `double`, 1-sigma energy resolution of Detector 1
- detector2_sigma: type `double`, 1-sigma energy resolution of Detector 2

**FillHistogram** defines histograms and fill the energies generated in the privious module. This defines parameters of these histograms:
- nbin: type `int`, number of bins
- energy_min: type `double`, lower bound of the histograms
- energy_max: type `double`, upper bound of the histograms

## Directory structure

You need only source and run directories. All other directories are not required to make an ANL application.

- source: the source tree defines ANL modules. Here is the main cmake file (CMakeLists.txt).
    * source/include: C++ header files (*.hh) declare the modules.
    * source/src: C++ source files (*.cc) define the modules.
    * source/rubyext: SWIG interface file to build a Ruby extension library.
- run: this directory a Ruby script (`run_test_histogram.rb`) that defines the ANL application. You can directly execute this script.
- reference_generation (optional): you can automatically make a reference document.
- script_generation (optional): you can automatically make a run script.

## How to build

    # pwd ===> /path/to/ANLNext/examples/histogram
    mkdir build
    cd build
    cmake ../source -DCMAKE_INSTALL_PREFIX=/path/to/install

By default, the install destination is your home directory.

    make
    make install

## How to run

    # pwd ===> /path/to/ANLNext/examples/histogram
    cd run
    ./run_test_histogram.rb

After the run, a ROOT file containing histograms should be generated. Check it!
