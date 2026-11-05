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
from copy import deepcopy

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
    if status==anlnext.ANLStatus_proceed:
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
        self.anl_manager = None
        self._stage = None
        self._applying_parameters = False


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
        return self.anl_manager.access_to_module(chain_id, module_id)


    def expose_module(self, module_id):
        mod = self.get_module(module_id)
        if mod is None:
            raise RuntimeError(f'ANL module {module_id} is not registered.')
        self.current_module = mod
        return self.current_module


    def insert_to_map(self, map_name, key, values):
        self._check_parameter_configuration()
        mod = self.current_module
        values = deepcopy(values)
        def parameter_setter():
            anlnext.insert_to_map(mod, map_name, key, values)
        self._reserve_parameter(parameter_setter)
        return self
      

    def push_to_vector(self, vector_name, values):
        self._check_parameter_configuration()
        mod = self.current_module
        values = deepcopy(values)
        def parameter_setter():
            anlnext.push_to_vector(mod, vector_name, values)
        self._reserve_parameter(parameter_setter)
        return self


    def set_parameter(self, name, value):
        self._check_parameter_configuration()
        mod = self.current_module
        value = deepcopy(value)
        def parameter_setter():
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
        self._reserve_parameter(parameter_setter)
        return self


    def _check_parameter_configuration(self):
        if self._stage not in (None, 'definition_done', 'pre_initialization_done'):
            raise RuntimeError(f'Parameter configuration is not allowed at this stage: {self._stage}.')


    def _reserve_parameter(self, setter):
        self._check_parameter_configuration()
        if self.current_module is None:
            raise RuntimeError('No current module is selected.')
        self.parameter_setter_list.append(setter)


    def set_parameters(self, parameters):
        self._check_parameter_configuration()
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


    with_parameters = set_parameters


    def set_module_parameter(self, module_id, name, value):
        self._check_parameter_configuration()
        self.expose_module(module_id)
        return self.set_parameter(name, value)


    def set_module_parameters(self, module_id, parameters):
        self._check_parameter_configuration()
        self.expose_module(module_id)
        return self.set_parameters(parameters)


    def load_parameters_from_yaml(self, filename):
        self._check_parameter_configuration()
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
                self.set_parameters(parameters)
        finally:
            self.current_module = current_module
        return self


    def with_submodule(self, name, key):
        return self.with_setter(lambda module: module.set_submodule_by_key(name, key))


    def with_setter(self, func):
        mod = self.current_module
        def parameter_setter():
            func(mod)
        self._reserve_parameter(parameter_setter)
        return self


    def modify(self, func):
        self.parameter_modifier = func


    def define(self):
        self._check_parameter_configuration()
        if self._stage is not None:
            return anlnext.ANLStatus_proceed
        try:
            if self.num_parallels > 1:
                self.anl_manager = anlnext.ANLManagerMT(self.num_parallels)
            else:
                self.anl_manager = anlnext.ANLManager()
            self.anl_manager.set_modules(self.module_list)
            self._stage = 'definition'
            status = self.anl_manager.define_analysis()
            if status == anlnext.ANLStatus_proceed:
                self._stage = 'definition_done'
            else:
                self._stage = 'error_to_terminate'
            return status
        except Exception:
            self._stage = 'error_to_terminate'
            raise


    def load_all_parameters(self):
        self._check_parameter_configuration()
        if self._stage is None:
            raise RuntimeError('Definition stage is not completed.')
        if self._applying_parameters:
            raise RuntimeError('Parameter application is already in progress.')
        try:
            self._applying_parameters = True
            while self.parameter_setter_list:
                self.parameter_setter_list[0]()
                self.parameter_setter_list.pop(0)
        except Exception:
            self._stage = 'error_to_terminate'
            raise
        finally:
            self._applying_parameters = False


    def run(self, num_loop):
        if self._stage not in (None, 'definition_done'):
            raise RuntimeError(f'Analysis cannot start at this stage: {self._stage}.')
        if self.display_period is None:
            self.display_period = proposed_display_period(num_loop)

        if num_loop == -1:
            num_loop = ctypes.c_size_t(-1).value # maximum value of size_t

        try:
            status = self.define()
            if not check_status(status, "define_analysis()"):
                return
            self.load_all_parameters()

            self._stage = 'pre_initialization'
            status = self.anl_manager.pre_initialize_analysis()
            if not check_status(status, "pre_initialize_analysis()"):
                self._stage = 'error_to_terminate'
                return
            self._stage = 'pre_initialization_done'

            if self.parameter_modifier is not None:
                self.parameter_modifier(self)
            self.load_all_parameters()

            self._stage = 'initialization'
            status = self.anl_manager.initialize_analysis()
            if not check_status(status, "initialize_analysis()"):
                self._stage = 'error_to_terminate'
                return
            self._stage = 'initialization_done'

            print("")
            print("<Begin Analysis> | Time: " + str(datetime.datetime.now()))
            sys.stdout.flush()
            self.anl_manager.set_display_period(self.display_period)

            status = self.anl_manager.run_analysis(num_loop, self.console)
            print("")
            print("<End Analysis>   | Time: " + str(datetime.datetime.now()))
            sys.stdout.flush()

            if not check_status(status, "run_analysis()"):
                print("Error: analyze() not successful")
            self._stage = 'analysis_done'

            status = self.anl_manager.finalize_analysis()
            if not check_status(status, "finalize_analysis()"):
                self._stage = 'error_to_terminate'
                return
            self._stage = 'finalization_done'
        except Exception:
            self._stage = 'error_to_terminate'
            raise
