# anlnext/chain.py
# Class library written in Python for the ANL Next framework.
#
# @author: Hirokazu Odaka
# @date: 2024-08-30
#

import anlnext.anlnextpy
import math
import datetime
import sys
import ctypes

#
# utility
#

def proposed_display_period(num_loop):
    if num_loop < 0:
        display_period = 10000
    elif num_loop < 100:
        display_period = 1
    else:
        display_period = 10**(int(math.log10(num_loop)-1.5))
    return display_period


def check_status(status, text):
    if status==anlnext.status.AS_OK:
        return True
    else:
        return False

#
# class
#

class AnalysisChain:
    def __init__(self):
        self.console = True
        self.num_parallels = 1
        self.display_period = None
        self.module_list = []
        self.current_module = None
        self.parameter_setter_list = []
        self.parameter_modifier = None


    def setup(self, func):
        func(self)
        return self


    def chain(self, anl_module_class, module_id=None):
        mod = anl_module_class()
        if module_id is not None:
            mod.set_module_id(module_id)
        self.module_list.append(mod)
        self.current_module = mod
        return self


    def get_module(self, module_id):
        for mod in self.module_list:
            if mod.module_id() == module_id:
                return mod
        else:
            return None


    def get_parallel_module(self, chain_id, module_id):
        return self.anl.access_to_module(chain_id, module_id)


    def expose_module(self, module_id):
        self.current_module = self.get_module(module_id)
        return self.current_module


    def insert_to_map(self, map_name, key, values):
        mod = self.current_module
        def set_param():
            anlnext.insert_to_map(mod, map_name, key, values)
        self.parameter_setter_list.append(set_param)
        return self
      

    def push_to_vector(self, vector_name, values):
        mod = self.current_module
        def set_param():
            anlnext.push_to_vector(mod, vector_name, values)
        self.parameter_setter_list.append(set_param)
        return self


    def set_parameter(self, name, value):
        mod = self.current_module
        def set_param():
            target = mod
            parameter_name = name
            if '.' in name:
                parts = name.split('.')
                if len(parts) != 2 or not all(parts):
                    raise ValueError(f'Invalid submodule parameter path: {name}')
                submodule_name, parameter_name = parts
                target = mod.get_submodule(submodule_name)
            if isinstance(value, dict):
                anlnext.set_map_parameter(target, parameter_name, value)
            elif isinstance(value, list):
                anlnext.set_vector_parameter(target, parameter_name, value)
            else:
                if isinstance(value, bool):
                    target.set_parameter(parameter_name, value)
                elif isinstance(value, int):
                    if target.get_parameter(parameter_name).type_name() == "unsigned_integer":
                        target.set_parameter_unsigned_integer(parameter_name, value)
                    else:
                        target.set_parameter_integer(parameter_name, value)
                else:
                    target.set_parameter(parameter_name, value)
        self.parameter_setter_list.append(set_param)
        return self


    def with_parameters(self, parameters):
        for name, value in parameters.items():
            if name.endswith('_') and '.' not in name:
                submodule_name = name[:-1]
                if not submodule_name or not isinstance(value, dict):
                    raise ValueError(f'Submodule parameter group requires a nonempty name and a dict: {name}')
                for parameter_name, parameter_value in value.items():
                    self.set_parameter(f'{submodule_name}.{parameter_name}', parameter_value)
            else:
                self.set_parameter(name, value)
        return self


    def load_parameters_from_yaml(self, filename):
        try:
            import yaml
        except ImportError as error:
            raise ImportError('YAML parameter files require PyYAML: install it with python -m pip install PyYAML') from error

        with open(filename, encoding='utf-8') as stream:
            parameters_by_module = yaml.safe_load(stream)
        if not isinstance(parameters_by_module, dict):
            raise ValueError('YAML parameter file must map module IDs to parameter mappings.')

        settings = []
        for module_id, parameters in parameters_by_module.items():
            if (not isinstance(module_id, str) or not isinstance(parameters, dict)
                    or not all(isinstance(name, str) for name in parameters)):
                raise ValueError('YAML module IDs and parameter names must be strings, with a mapping for each module.')
            mod = self.get_module(module_id)
            if mod is None:
                raise ValueError(f'Unknown module ID in YAML parameter file: {module_id}')
            settings.append((mod, parameters))

        current_module = self.current_module
        try:
            for mod, parameters in settings:
                self.current_module = mod
                self.with_parameters(parameters)
        finally:
            self.current_module = current_module
        return self


    def with_submodule(self, name, key):
        return self.with_setter(lambda module: module.set_submodule_by_key(name, key))


    def with_setter(self, func):
        mod = self.current_module
        def set_param():
            func(mod)
        self.parameter_setter_list.append(set_param)
        return self


    def modify(self, func):
        self.parameter_modifier = func


    def define(self):
        if self.num_parallels > 1:
            self.anl = anlnext.ANLManagerMT(self.num_parallels)
        else:
            self.anl = anlnext.ANLManager()
        self.anl.set_modules(self.module_list)
        return self.anl.define_analysis()


    def load_all_parameters(self):
        for f in self.parameter_setter_list:
            f()
        self.parameter_setter_list.clear()


    def run(self, num_loop):
        if self.display_period is None:
            self.display_period = proposed_display_period(num_loop)

        if num_loop == -1:
            num_loop = ctypes.c_size_t(-1).value # maximum value of size_t

        status = self.define()
        if not check_status(status, "define_analysis()"):
            return
        self.load_all_parameters()

        status = self.anl.pre_initialize_analysis()
        if not check_status(status, "pre_initialize_analysis()"):
            return

        if self.parameter_modifier is not None:
            self.parameter_modifier(self)

        status = self.anl.initialize_analysis()
        if not check_status(status, "initialize_analysis()"):
            return

        print("")
        print("<Begin Analysis> | Time: " + str(datetime.datetime.now()))
        sys.stdout.flush()
        self.anl.set_display_period(self.display_period)

        status = self.anl.run_analysis(num_loop, self.console)
        print("")
        print("<End Analysis>   | Time: " + str(datetime.datetime.now()))
        sys.stdout.flush()

        if not check_status(status, "run_analysis()"):
            print("Error: analyze() not successful")

        status = self.anl.finalize_analysis()
        if not check_status(status, "finalize_analysis()"):
            return
