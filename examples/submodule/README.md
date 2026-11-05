# Submodule example

Select an algorithm in Ruby or Python, set its parameters, print them, and
read their values. This example requires no ROOT or YAML library.

`AlgorithmModule` declares a submodule named `algorithm` and implements
`set_submodule_by_key()` to select either implementation:

- `multiply`: `MultiplyAlgorithm` multiplies the input by `factor`.
- `offset`: `OffsetAlgorithm` adds `offset` to the input.

Both implementations use the parameter registry and Evs services of
`BasicSubModule`. The parent decides when to call the calculation method.

## Build and run

First build and install ANLNext version 3 with its Ruby and Python libraries.
From this directory, use the same installation prefix:

```sh
cmake -S source -B build \
  -DANLNEXT_INSTALL=/path/to/anlnext \
  -DCMAKE_INSTALL_PREFIX=/path/to/anlnext
cmake --build build
cmake --install build
```

Add the installation's `lib/ruby` and `lib/python` directories to `RUBYLIB`
and `PYTHONPATH`, respectively. Run either script with an optional chain
count (default: 3):

```sh
ruby run/run_submodule.rb 3
python3 run_py/run_submodule.py 3
```

The ANLNext CTest build also builds and runs this example without installing it.
Ten iterations with input 4 give totals of 280 for `factor = 7` and 70 for
`offset = 3`, with either one or three chains.

The same example can read values from `parameters.yaml`:

```sh
ruby run/run_submodule.rb 3 parameters.yaml
python3 run_py/run_submodule.py 3 parameters.yaml
```

The Python YAML reader requires PyYAML (`python3 -m pip install PyYAML`).
Submodule implementations are still selected by the script. Only parameter
values move into the YAML file; omit the filename to use the inline settings.
See [YAML parameter files](../../documentation/yaml_parameters_v3.md) for details.

## Select and configure

Ruby:

```ruby
chain.chain(SubmoduleExample::AlgorithmModule, :multiply)
chain.with_submodule('algorithm', 'multiply')
chain.with_parameters(input: 4.0, "algorithm.factor": 7.0)
```

Python:

```python
chain.chain(AlgorithmModule, 'multiply')
chain.with_submodule('algorithm', 'multiply')
chain.with_parameters({'input': 4.0, 'algorithm.factor': 7.0})
```

`with_submodule()` selects the instance after module definition, before
parameter settings are applied. Setters run in registration order, so place
selection immediately after `chain()`. Selecting a submodule preserves the
current parent module.

`with_parameters()` accepts both parent parameters and `submodule.parameter`
paths. The dot separates the submodule name from its parameter name; neither
name can contain a dot in this form.
Ruby accepts a quoted symbol key such as `"algorithm.factor": 7.0`, or a string
key such as `'algorithm.factor' => 7.0`. Scalar, vector, and map values use
the same setting operations as parent parameters.

Submodule parameters can also be grouped under a key ending in `_`:

```ruby
chain.with_parameters(input: 4.0, algorithm_: {factor: 7.0})
```

```python
chain.with_parameters({'input': 4.0, 'algorithm_': {'factor': 7.0}})
```

This is equivalent to the dotted form above. At the top level, a key without
a dot that ends in `_` denotes a submodule group and requires a Hash/dict.
Remove only the final `_` to obtain the submodule name. Other underscores
remain ordinary characters. A group contains parameter values, including
vectors and maps; its contents are not interpreted as further groups.
Ordinary map parameters use keys without the trailing `_`, and their map
entries are passed through unchanged. An empty group applies no settings.
Both forms can be mixed in the same call and are applied in entry order.

Ruby and Python both queue settings until `load_all_parameters()`, including
settings registered after definition. Normal `run()` applies them before
pre-initialization and chain duplication, and again after `modify` before
initialization. To read a newly configured value during setup, explicitly
call `load_all_parameters()` after definition. Initialization closes the
Chain configuration API; direct module setters remain immediate operations.

## Display and read

After settings have been applied, use the same registry operations as for
a regular module:

```ruby
algorithm = chain.get_module(:multiply).get_submodule('algorithm')
algorithm.print_parameters
factor = algorithm.get_parameter_value('factor')
information = algorithm.parameters_to_object
```

```python
algorithm = chain.get_module('multiply').get_submodule('algorithm')
algorithm.print_parameters()
factor = anlnext.get_value(algorithm.get_parameter('factor'))
```

Normal initialization also prints the submodule's parameters. Ruby's
`parameters_to_object()` and the manager's JSON output include submodules.
Unknown or unset submodules and unknown parameters raise the existing errors.
Submodule names and output follow declaration order. Replacing an instance
preserves its position, and parallel copies retain the same order.

## Parallel execution

The framework duplicates the selected submodule and its parameters with
each parent. Evs and module access are connected to the corresponding chain.
The algorithm selection is fixed before duplication; parameter values are
independent between copies.

The parent connects its pointer member to the registered name in `mod_define()`:

```cpp
define_submodule("algorithm", &AlgorithmModule::algorithm_);
```

The framework sets `algorithm_` when an instance is registered or replaced,
and reconnects the corresponding member after duplication. It is null while
the declared slot is unset. The parent needs no lookup or cast in
`mod_initialize()` or `mod_analyze()`. Ownership remains with the framework.
An incompatible instance raises `SubModuleBindingError`.

The name-only `define_submodule("algorithm")` is also supported for code
that obtains its own references. References not connected by declaration
remain application state. Replacing an instance invalidates other references
to the old one.

`mod_begin_run()` resets the parent's results. `mod_merge()` sums the
per-chain totals, demonstrating a policy compatible with repeated runs.
