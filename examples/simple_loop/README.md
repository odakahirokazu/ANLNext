# simple_loop

This is one of the simplest example of ANL application.
This executes a simple loop, doing nothing in the analysis stage.
In modules in this example, you can see how to define module parameters.

**MyModule** defines six basic-type parameters:
- my_parameter1: type `int`
- my_parameter2: type `double`
- my_parameter3: type `std::string`
- my_vector1: type `std::vector<int>`
- my_vector2: type `std::vector<double>`
- my_vector3: type `std::vector<std::string>`

**MyVectorModule** defines a complex vector-type parameter:
- my_vector: type `std::vector<std::tuple<int, std::string, double, double>>`

**MyMapModule** defines a complex map-type parameter:
- my_map: type `std::map<std::string, std::tuple<int, std::string, double, double>>`

## Directory structure

You need only source and run directories. All other directories are not required to make an ANL application.

- source: the source tree defines ANL modules. Here is the main cmake file (CMakeLists.txt).
    * source/include: C++ header files (*.hh) declare the modules.
    * source/src: C++ source files (*.cc) define the modules.
    * source/rubyext: SWIG interface file to build a Ruby extension library.
- run: this directory a Ruby script (`run_simple_loop.rb`) that defines the ANL application. You can directly execute this script.
- run_interactive (optional): a run script in an interactive mode
- reference_generation (optional): you can automatically make a reference document.
- script_generation (optional): you can automatically make a run script.
- misc (optional, technical): shows how to examine the modules.

## How to build

    # pwd ===> /path/to/ANLNext/examples/simple_loop
    mkdir build
    cd build
    cmake ../source -DCMAKE_INSTALL_PREFIX=/path/to/install

By default, the install destination is your home directory.

    make
    make install

## How to run

    # pwd ===> /path/to/ANLNext/examples/simple_loop
    cd run
    ./run_simple_loop.rb
