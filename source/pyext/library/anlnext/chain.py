# anlnext/chain.py
# Class library written in Python for the ANL Next framework.
#
# @author: Hirokazu Odaka
# @date: 2024-08-30
#

import anlnext
import math
import datetime
import sys

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
        self.display_period = None
        self.module_list = []
        self.current_module = None
        self.parameter_setter_list = []


    def chain(self, anl_module_class, module_id=None):
        mod = anl_module_class()
        if module_id:
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


    def expose_module(self, module_id):
        self.current_module = self.get_module(module_id)
        return self.current_module


    def insert_to_map(self, map_name, key, values):
        mod = self.current_module
        def set_param():
            mod.insert_to_map(map_name, key, values)
        self.parameter_setter_list.append(set_param)
        return self
      

    def push_to_vector(self, vector_name, values):
        mod = self.current_module
        def set_param():
            mod.push_to_vector(vector_name, values)
        self.parameter_setter_list.append(set_param)
        return self


    def set_parameter(self, name, value):
        mod = self.current_module
        if isinstance(value, list):
            if len(value)>0:
                v0 = value[0]
                if isinstance(v0, str):
                    def set_param():
                        mod.set_parameter_vector_str(name, value)
                    self.parameter_setter_list.append(set_param)
                elif isinstance(v0, float):
                    def set_param():
                        mod.set_parameter_vector_d(name, value)
                    self.parameter_setter_list.append(set_param)
                elif isinstance(v0, int):
                    def set_param():
                        mod.set_parameter_vector_i(name, value)
                    self.parameter_setter_list.append(set_param)
        else:
            if isinstance(value, int):
                def set_param():
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


    def define(self):
        self.anl = anlnext.ANLManager()
        self.anl.set_modules(self.module_list)
        self.anl.Define()


    def load_all_parameters(self):
        for f in self.parameter_setter_list:
            f()
        self.parameter_setter_list.clear()


    def run(self, num_loop):
        if self.display_period==None:
            self.display_period = proposed_display_period(num_loop)

        self.define()
        self.load_all_parameters()
        status = self.anl.PreInitialize()
        if not check_status(status, "PreInitialize()"):
            return
        status = self.anl.Initialize()
        if not check_status(status, "Initialize()"):
            return

        print("")
        print("<Begin Analysis> | Time: " + str(datetime.datetime.now()))
        sys.stdout.flush()
        self.anl.set_display_period(self.display_period)

        status = self.anl.Analyze(num_loop, self.console)
        print("")
        print("<End Analysis>   | Time: " + str(datetime.datetime.now()))
        sys.stdout.flush()

        if not check_status(status, "Analyze()"):
            print("Error: Analyze() not successful")

        status = self.anl.Finalize()
        if not check_status(status, "Finalize()"):
            return
