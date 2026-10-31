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
        if isinstance(value, dict):
            def set_param():
                anlnext.set_map_parameter(mod, name, value)
            self.parameter_setter_list.append(set_param)
        elif isinstance(value, list):
            def set_param():
                anlnext.set_vector_parameter(mod, name, value)
            self.parameter_setter_list.append(set_param)
        else:
            if isinstance(value, bool):
                def set_param():
                    mod.set_parameter(name, value)
                self.parameter_setter_list.append(set_param)
            elif isinstance(value, int):
                def set_param():
                    if mod.get_parameter(name).type_name() == "unsigned_integer":
                        mod.set_parameter_unsigned_integer(name, value)
                    else:
                        mod.set_parameter_integer(name, value)
                self.parameter_setter_list.append(set_param)
            else:
                def set_param():
                    mod.set_parameter(name, value)
                self.parameter_setter_list.append(set_param)
        return self


    def with_parameters(self, parameters):
        mod = self.current_module
        for name, value in parameters.items():
            self.set_parameter(name, value)
        return self


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
        self.anl.define_analysis()


    def load_all_parameters(self):
        for f in self.parameter_setter_list:
            f()
        self.parameter_setter_list.clear()


    def run(self, num_loop):
        if self.display_period is None:
            self.display_period = proposed_display_period(num_loop)

        if num_loop == -1:
            num_loop = ctypes.c_size_t(-1).value # maximum value of size_t

        self.define()
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
