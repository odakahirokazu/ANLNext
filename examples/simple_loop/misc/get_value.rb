#!/usr/bin/env ruby

require 'ANLLib' # ANL Next library
require 'myPackage'# Ruby extension library using ANL Next

# Define your own application class derived from ANL::ANLApp.
class MyApp < ANL::ANLApp
  # Define an analysis chain in setup() method.
  def setup()
    chain MyPackage::MyModule
    with_parameters(MyParameter1: 10,
                    MyParameter2: 20.5,
                    MyParameter3: "Hello",
                    MyVector1: [1, 2, 3, 4, 5],
                    MyVector2: [1.3, 4.0, 11.2, 3.2],
                    MyVector3: ["Hakuba", "Niseko", "Appi"])

    chain MyPackage::MyMapModule
    with_parameters()
    insert_to_map :my_map, "Si1",   {ID: 1, type: "strip", x: 0.0, y: 0.0}
    insert_to_map :my_map, "Si2",   {ID: 2, type: "strip", x: 0.0, y: 2.0}
    insert_to_map :my_map, "CdTe1", {ID: 3, type: "pixel", x: -0.2, y: 4.0}
    insert_to_map :my_map, "CdTe2", {ID: 4, type: "pixel", x: -0.2, y: 6.0}

    chain MyPackage::MyVectorModule
    with_parameters()
    push_to_vector :my_vector, {ID: 1, type: "strip", x: 0.0, y: 0.0}
    push_to_vector :my_vector, {ID: 3, type: "pixel", x: -0.2, y: 4.0}
    push_to_vector :my_vector, {ID: 4, type: "pixel", x: -0.2, y: 6.0}

    # If you need to add the same type of module, you should set another name
    # via the second argument.
    chain MyPackage::MyModule, :MyModule2
    with_parameters(MyParameter2: 102.1,
                    MyVector3: ["Jupiter", "Venus", "Mars", "Saturn"])
  end
end

def print_params(mod)
  mod.parameter_list.each do |param|
    type = param.type_name
    case type
    when "vector"
      puts "#{param.name} #{param.type_name} #{param.get_value_auto}"
      param.size_of_container.times do |k|
        puts "  index: #{k}"
        param.retrieve_from_container(k)
        param.num_value_elements.times do |i|
          value_element = param.value_element_info(i)
          puts "    #{value_element.name} #{value_element.type_name} #{value_element.get_value_auto}"
        end
      end
    when "map"
      puts "#{param.name} #{param.type_name} #{param.get_value_auto}"
      param.map_key_list.each do |k|
        puts "  key: #{k}"
        param.retrieve_from_container(k)
        param.num_value_elements.times do |i|
          value_element = param.value_element_info(i)
          puts "    #{value_element.name} #{value_element.type_name} #{value_element.get_value_auto}"
        end
      end
    else
      puts "#{param.name} #{param.type_name} #{param.get_value_auto}"
    end
  end
end

a = MyApp.new

a.setup
a.startup
a.prepare_all_parameters
a.print_all_parameters

[:MyModule, :MyMapModule, :MyVectorModule, :MyModule2].each do |module_id|
  puts "ANL Module #{module_id}"
  print_params(a.get_module(module_id))
  puts ""
end

p a.get_module(:MyModule).get_parameter(:MyParameter3).get_value_auto
