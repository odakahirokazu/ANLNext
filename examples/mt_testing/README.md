# mt_testing

This is a simple example of multi-thread mode of ANL application.

## Directory structure

- source: the source tree defines ANL modules. Here is the main cmake file (CMakeLists.txt).
    * source/include: C++ header files (*.hh) declare the modules.
    * source/src: C++ source files (*.cc) define the modules.
    * source/rubyext: SWIG interface file to build a Ruby extension library.
- run: this directory a Ruby script (`run_simple_loop.rb`) that defines the ANL application. You can directly execute this script.

## How to build

    # pwd ===> /path/to/ANLNext/examples/mt_testing
    mkdir build
    cd build
    cmake ../source -DCMAKE_INSTALL_PREFIX=/path/to/install

By default, the install destination is your home directory.

    make
    make install

## How to run

    # pwd ===> /path/to/ANLNext/examples/mt_testing
    cd run
    ./run_mt_test.rb
