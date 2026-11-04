# anlnext/basic.rb
# Class library written in Ruby for the ANL Next framework.
#
# @author: Hirokazu Odaka
#

module ANL
  ### aliases of ANL status
  AS_OK             = ANL::ANLStatus_ok
  AS_SKIP           = ANL::ANLStatus_skip
  AS_QUIT           = ANL::ANLStatus_quit

  MAP_PARAMETER_TYPES = {
    'map<int,int>' => 'i_i',
    'map<int,double>' => 'i_d',
    'map<int,string>' => 'i_str',
    'map<string,int>' => 'str_i',
    'map<string,double>' => 'str_d',
    'map<string,string>' => 'str_str',
  }.freeze

  class ParameterRegistry
    alias :set_parameter_original :set_parameter
    def set_parameter(name, *values)
      if values.size == 1 && values.first.is_a?(Hash)
        type = get_parameter(name).type_name()
        suffix = MAP_PARAMETER_TYPES[type]
        raise TypeError, "#{name} is not a map parameter: #{type}" unless suffix
        public_send("set_parameter_map_#{suffix}", name.to_s, values.first)
      elsif values.size == 1 && values.first.is_a?(Integer) &&
            get_parameter(name).type_name() == 'unsigned_integer'
        set_parameter_unsigned_integer(name.to_s, values.first)
      else
        set_parameter_original(name.to_s, *values)
      end
    end

    # Get a list of parameters registered in the module.
    #
    # @return [Array] list of parameters registered in the given module.
    #
    def parameter_list()
      l = []
      i = self.parameter_begin
      e = self.parameter_end
      while i != e
        l << i.value.__deref__
        i.next
      end
      return l
    end

    alias :get_parameter_original :get_parameter
    def get_parameter(name)
      get_parameter_original(name.to_s)
    end

    def get_parameter_value(name)
      get_parameter(name).to_value_object()
    end
    alias :get_result_value :get_parameter_value
  end

  class BasicSubModule
    def parameters_to_object()
      { name: submodule_name(), version: submodule_version(),
        parameter_list: parameter_list.map(&:to_object) }
    end
  end

  class BasicModule
    def parameters_to_object()
      o = {}
      o[:module_id] = module_id()
      o[:name] = module_name()
      o[:version] = module_version()
      o[:parameter_list] = parameter_list.map(&:to_object)
      names = submodule_names()
      unless names.empty?
        o[:submodule_list] = names.map do |name|
          if is_submodule_set(name)
            get_submodule(name).parameters_to_object.merge(registered_name: name, state: 'set')
          else
            { registered_name: name, state: 'unset' }
          end
        end
      end
      return o
    end
  end

  class VModuleParameter
    def get_value_auto()
      if (suffix = MAP_PARAMETER_TYPES[type_name()])
        return public_send("get_value_map_#{suffix}", {})
      end
      case type_name()
      when "bool"
        get_value(false)
      when "int"
        get_value(0)
      when "integer"
        get_value_integer()
      when "unsigned_integer"
        get_value_unsigned_integer()
      when "double"
        get_value(0.0)
      when "string"
        get_value('')
      when "vector<int>"
        get_value_vector_i([])
      when "vector<double>"
        get_value_vector_d([])
      when "vector<string>"
        get_value_vector_str([])
      when "2-vector"
        get_value(0.0, 0.0)
      when "3-vector"
        get_value(0.0, 0.0, 0.0)
      else
        nil
      end
    end

    def to_value_object(children_into_hash: true, value_element_to_object: false)
      case type_name()
      when "record_vector"
        values = []
        size_of_container.times do |k|
          values <<
            if children_into_hash
              get_hash_from_container(k, value_element_to_object: value_element_to_object)
            else
              get_list_from_container(k, value_element_to_object: value_element_to_object)
            end
        end
        return values
      when "record_map"
        values = {}
        map_key_list.each do |k|
          values[k] =
            if children_into_hash
              get_hash_from_container(k, value_element_to_object: value_element_to_object)
            else
              get_list_from_container(k, value_element_to_object: value_element_to_object)
            end
        end
        return values
      else
        return get_value_auto()
      end
    end

    def to_object(children_into_hash: false)
      type = type_name()
      o = {}
      o[:name] = name()
      o[:type] = type

      case type
      when "double", "vector<double>", "map<int,double>", "map<string,double>"
        o[:unit] = unit()
        o[:unit_name] = unit_name()
      end

      o[:value] = self.to_value_object(children_into_hash: children_into_hash,
                                       value_element_to_object: true)
      if MAP_PARAMETER_TYPES.key?(type)
        o[:value] = o[:value].map { |key, value| { key: key, value: value } }
      end
      return o
    end

    def get_list_from_container(k, value_element_to_object: false)
      o = []
      value_elements_from_container(k).each do |element|
        if value_element_to_object
          o.push(element.to_object)
        else
          o.push(element.to_value_object)
        end
      end
      return o
    end

    def get_hash_from_container(k, value_element_to_object: false)
      o = {}
      value_elements_from_container(k).each do |element|
        if value_element_to_object
          o[element.name] = element.to_object
        else
          o[element.name] = element.to_value_object
        end
      end
      return o
    end
  end

  class SPtrModParam
    def to_object(**options)
      __deref__.to_object(**options)
    end

    def to_value_object(**options)
      __deref__.to_value_object(**options)
    end
  end

  # Class of two- or three-dimensional vector.
  #
  class Vector
    def initialize(x, y, z=nil)
      @x = x
      @y = y
      @z = z
    end

    attr_reader :x, :y, :z

    def set(x, y, z=nil)
      @x = x
      @y = y
      @z = z
    end
  end

  # Class of ANL module + its parameters
  #
  class ModuleInitializer
    def initialize(anl_module_class, module_id=nil)
      @module_class = anl_module_class
      @module_id = module_id
      @methods_to_play = []
    end
    attr_accessor :module_id
    attr_reader :module_class

    def keep_method(method, args, &block)
      m = lambda do |receiver|
        receiver.send(method, *args, &block)
      end
      @methods_to_play << m
    end

    [:with_parameters, :with, :push_to_vector, :insert_to_map].each do |m|
      define_method(m) do |*args, &block|
        keep_method(m, args, &block)
      end
    end

    def play_methods(receiver)
      @methods_to_play.each do |m|
        m.(receiver)
      end
    end
  end

end # module ANL
