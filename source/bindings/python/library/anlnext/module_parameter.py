# anlnext/module_parameter.py
#
# @author: Hirokazu Odaka
# @date: 2024-08-30
#

import anlnext.anlnextpy

_map_types = {
    "map<int,int>": "i_i",
    "map<int,double>": "i_d",
    "map<int,string>": "i_str",
    "map<string,int>": "str_i",
    "map<string,double>": "str_d",
    "map<string,string>": "str_str",
}


def set_map_parameter(module, name, value):
    parameter_type = module.get_parameter(name).type_name()
    if parameter_type not in _map_types:
        raise TypeError(f"{name} is not a map parameter: {parameter_type}")
    suffix = _map_types[parameter_type]
    getattr(module, "set_parameter_map_" + suffix)(name, value)


def get_value(param):
    if param.type_name() in _map_types:
        return dict(getattr(param, "get_value_map_" + _map_types[param.type_name()])({}))
    if param.type_name()=="bool":
        return param.get_value(False)
    elif param.type_name()=="int":
        return param.get_value(0)
    elif param.type_name()=="integer":
        return param.get_value_integer()
    elif param.type_name()=="unsigned_integer":
        return param.get_value_unsigned_integer()
    elif param.type_name()=="double":
        return param.get_value(0.0)
    elif param.type_name()=="string":
        return param.get_value('')
    elif param.type_name()=="vector<int>":
        return param.get_value_vector_i([])
    elif param.type_name()=="vector<double>":
        return param.get_value_vector_d([])
    elif param.type_name()=="vector<string>":
        return param.get_value_vector_str([])
    elif param.type_name()=="2-vector":
        return param.get_value(0.0, 0.0)
    elif param.type_name()=="3-vector":
        return param.get_value(0.0, 0.0, 0.0)
    elif param.type_name()=="record_vector":
        return get_vector_value(param)
    elif param.type_name()=="record_map":
        return get_map_value(param)
    else:
        return None


def get_vector_value(param):
    values = []
    for k in range(param.size_of_container()):
        v = get_dict_from_container(param, k)
        values.append(v)
    return values


def get_map_value(param):
    values = {}
    for k in param.map_key_list():
        v = get_dict_from_container(param, k)
        values[k] = v
    return values


def get_dict_from_container(param, k):
    o = {}
    param.retrieve_from_container(k)
    for i in range(param.num_value_elements()):
        element = param.value_element_info(i)
        o[element.name()] = get_value(element)
    return o
