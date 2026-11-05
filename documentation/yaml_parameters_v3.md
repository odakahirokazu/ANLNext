# YAML parameter files

Ruby and Python analysis chains accept parameter values from a YAML file:

```ruby
chain.chain(MyPackage::Processor, :processor)
chain.with_submodule('algorithm', 'multiply')
chain.load_parameters_from_yaml('parameters.yaml')
chain.run(100)
```

```python
chain.chain(Processor, 'processor')
chain.with_submodule('algorithm', 'multiply')
chain.load_parameters_from_yaml('parameters.yaml')
chain.run(100)
```

Ruby's `ANLApp` also delegates `load_parameters_from_yaml()` to its analysis
chain. Build the chain and select its submodule implementations before loading
the file. The file sets values; it does not construct modules or submodules.

## File structure

```yaml
processor:
  input: 4.0
  algorithm_:
    factor: 7.0
```

The top-level keys are existing `module_id` strings. Each value is a mapping
of parameter names to values, passed to the existing `with_parameters()` API.
The file contains one YAML document. An empty mapping (`{}`) makes no changes;
an empty file, a null value, or a sequence at the top level is an error.

Submodule parameters accept either the grouped form above or dotted paths:

```yaml
processor:
  input: 4.0
  algorithm.factor: 7.0
```

A key ending in `_` without a dot denotes a submodule group. Remove only the
final underscore to obtain its registered name. A group contains parameter
values, including vectors and maps; further submodule nesting is not supported.
Both forms may appear together and follow the same rules as scripted settings.

Ordinary map parameters have no trailing `_`. Their entries are values, so
dots and underscores inside map keys have no special meaning:

```yaml
processor:
  samples: [2, 3, 5]
  weights: {left_: 0.5, a.b: 1.5}
  channels: {0: "input", 1: "output"}
```

The supported parameter values are those accepted by `with_parameters()` in
each language: scalars, scalar vectors, and scalar maps. The Ruby API also
accepts its existing `record_map` input form. This reader adds no new
`record_vector` or `record_map` representation; existing insertion APIs remain
available for records.

Use YAML integers for integer parameters and map keys, and decimal values
such as `7.0` for floating-point parameters. Large unsigned integers can be
written in decimal or hexadecimal, for example `0xffffffffffffffff`.
Values use the same parameter units as scripted settings.

Use `true`/`false` for booleans. Quote strings that YAML might interpret as
other types, including `"on"`, `"off"`, `"2026-11-05"`, and numeric strings.
Module IDs and parameter names must be strings as well. YAML anchors and
aliases are accepted, for example to reuse a mapping of algorithm parameters.
Avoid duplicate keys in a mapping; ordinary parser duplicate-key behavior
applies. Prefer `1.0e+3` when writing floating-point values in scientific notation.

## Application order and errors

`load_parameters_from_yaml()` reads the file when called. Ruby and Python both
queue settings in file entry order, interleaved with scripted setters in call
order. This also applies after definition. Later value assignments override
earlier ones; record insertion retains its existing insertion behavior.
The selected current module is restored after loading, including when an error
occurs.

Normal `run()` applies queued settings before pre-initialization and chain
duplication, so parallel copies receive the configured values. It applies any
further settings registered by `modify` before initialization. During setup,
call `load_all_parameters()` explicitly after definition to apply pending
settings before reading values. Reading or displaying values does not apply
queued setters.

The Chain setting APIs reject reservations once initialization starts. Direct
module setters remain immediate operations. A successful reservation is consumed
once; an empty queue makes no changes. If a setter fails, its exception propagates
and the chain cannot resume parameter application or analysis. Previously applied
values are retained; automatic retry and rollback are not provided.

The reader checks the top-level structure, parameter-name types, and all module
IDs before queuing settings. Unknown/unset submodules, unknown
parameters, and invalid parameter values raise the existing setting errors
when their setters execute. Configuration is not transactional; abort setup
if an error is raised.

## Dependencies and tests

Ruby uses its YAML/Psych library. Python requires
[PyYAML](https://pyyaml.org/wiki/PyYAMLDocumentation):

```sh
python3 -m pip install PyYAML
```

Both readers use safe loading. PyYAML is imported only when reading a YAML
file; other Python operations do not require it. The binding test suite
includes YAML tests and therefore requires PyYAML in its Python interpreter.
To use a virtual environment for those tests, configure CMake with
`-DPython3_EXECUTABLE=/path/to/venv/bin/python`.

The [submodule example](../examples/submodule/README.md) runs with either inline
settings or a shared YAML file, in Ruby/Python and with one or three chains.
