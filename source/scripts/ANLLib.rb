# ANLLib
# Class library written in Ruby for the ANL Next framework.
#
# @author: Hirokazu Odaka
#

require 'ANL'
require 'forwardable'
require 'rexml/document'
require 'json'

module ANL
  ### aliases of ANL status
  AS_OK             = ANL::ANLStatus_ok
  AS_SKIP           = ANL::ANLStatus_skip
  AS_SKIP_ERROR     = ANL::ANLStatus_skip_error
  AS_QUIT           = ANL::ANLStatus_quit
  AS_QUIT_ERROR     = ANL::ANLStatus_quit_error
  AS_QUIT_ALL       = ANL::ANLStatus_quit_all
  AS_QUIT_ALL_ERROR = ANL::ANLStatus_quit_all_error

  class BasicModule
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

    def parameters_to_object()
      o = {}
      o[:module_id] = module_id()
      o[:name] = module_name()
      o[:version] = module_version()
      o[:parameter_list] = parameter_list.map(&:to_object)
      return o
    end
  end

  class VModuleParameter
    def get_value_auto()
      case type_name()
      when "bool"
        get_value(false)
      when "int"
        get_value(0)
      when "integer"
        get_value_integer()
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
      when "vector"
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
      when "map"
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
      when "double", "vector<double>"
        o[:unit] = unit()
        o[:unit_name] = unit_name()
      end

      o[:value] = self.to_value_object(children_into_hash: children_into_hash,
                                       value_element_to_object: true)
      return o
    end

    def get_list_from_container(k, value_element_to_object: false)
      o = []
      retrieve_from_container(k)
      num_value_elements.times do |i|
        element = value_element_info(i)
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
      retrieve_from_container(k)
      num_value_elements.times do |i|
        element = value_element_info(i)
        if value_element_to_object
          o[element.name] = element.to_object
        else
          o[element.name] = element.to_value_object
        end
      end
      return o
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


  # Convert ANL status interger to string.
  #
  # @param [Int] i ANL status ID
  # @return [String] ANL status
  def show_status(i)
    {
      ANL::ANLStatus_ok => "AS_OK",
      ANL::ANLStatus_skip => "AS_SKIP",
      ANL::ANLStatus_skip_error => "AS_SKIP_ERROR",
      ANL::ANLStatus_quit => "AS_QUIT",
      ANL::ANLStatus_quit_error => "AS_QUIT_ERROR",
      ANL::ANLStatus_quit_all => "AS_QUIT_ALL",
      ANL::ANLStatus_quit_all_error => "AS_QUIT_ALL_ERROR",
    }[i] or "unknown status"
  end
  module_function :show_status


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


  # Class of an ANL Application.
  #
  class ANLApp
    extend Forwardable
    def initialize()
      @_anlapp_analysis_chain = AnalysisChain.new
    end

    # Setup analysis chain and set parameters.
    # By default, this method does nothing.
    # A user can redefine this method in a derived class.
    # This method is called at the beginning of run() method.
    #
    def setup()
      nil
    end

    def write_parameters_to_json(filename, master: true)
      @_anlapp_analysis_chain.parameters_json_filename = filename
      @_anlapp_analysis_chain.parameters_json_master = master
    end

    # Run this application.
    #
    # @param [Integer] num_loop number of loops. :all or -1 for infinite loops.
    # @param [Integer] display_frequency frequency of displaying loop ID
    #     to STDOUT.
    #
    def run(num_loop, display_frequency=nil)
      if @_anlapp_analysis_chain.finalization_done?
        @_anlapp_analysis_chain.clear()
      end

      if @_anlapp_analysis_chain.chain_empty?
        setup()
      end

      if display_frequency
        @_anlapp_analysis_chain.display_frequency = display_frequency
      end

      @_anlapp_analysis_chain.run(num_loop)
    end

    # Run this application in interactive mode.
    #
    def run_interactive()
      if @_anlapp_analysis_chain.finalization_done?
        @_anlapp_analysis_chain.clear()
      end

      if @_anlapp_analysis_chain.chain_empty?
        setup()
      end

      @_anlapp_analysis_chain.run_interactive()
    end

    ### method delegation to the AnalysisChain object.
    anlapp_methods = [
      :console, :console=, :add_namespace,
      :push, :insert, :insert_before, :insert_after, :chain,
      :expose_module, :get_module, :index, :rindex,
      :insert_to_map, :push_to_vector,
      :set_parameters, :with_parameters, :chain_with_parameters,
      :modify, :modify_parameters,
      :define, :load_all_parameters,
      :print_all_parameters, :parameters_to_object, :make_doc,
      :num_parallels, :num_parallels=,
      :display_frequency=,
    ]
    def_delegators :@_anlapp_analysis_chain, *anlapp_methods
    alias :with :with_parameters

    def self.define_setup_module(basename, module_class=nil, array: false)
      if array
        setter_name = "add_#{basename}".to_sym
        getter_name = "module_list_of_#{basename}".to_sym
        variable_name = "@_anlapp_module_list_#{basename}".to_sym
      else
        setter_name = "set_#{basename}".to_sym
        getter_name = "module_of_#{basename}".to_sym
        variable_name = "@_anlapp_module_#{basename}".to_sym
      end

      set_function = lambda do |receiver, x, y, array_type|
        if array_type
          if list = receiver.instance_variable_get(x)
            list << y
          else
            receiver.instance_variable_set(x, [y])
          end
        else
          receiver.instance_variable_set(x, y)
        end
      end

      def resolve_arguments(args)
        if args.size > 2
          raise "Too many arguments. The arguments number should be <= 2."
        else
          if args[0].kind_of?(String) || args[0].kind_of?(Symbol)
            module_id = args[0]
            parameters = args[1]
          else
            module_id = nil
            parameters = args[0]
          end
        end
        return module_id, parameters
      end

      if module_class
        define_method(setter_name) do |*args, &set_param|
          module_id, parameters = resolve_arguments(args)
          mi = ModuleInitializer.new(module_class, module_id)
          set_function.(self, variable_name, mi, array)
          @_anlapp_analysis_chain.current_module = mi
          mi.with(parameters, &set_param)
          mi
        end
      else
        define_method(setter_name) do |mod1, *args, &set_param|
          module_id, parameters = resolve_arguments(args)
          mi = ModuleInitializer.new(mod1, module_id)
          set_function.(self, variable_name, mi, array)
          @_anlapp_analysis_chain.current_module = mi
          mi.with(parameters, &set_param)
          mi
        end
      end

      define_method(getter_name){ instance_variable_get(variable_name) }
    end

    def make_script(output: nil,
                    package: "myPackage",
                    namespace: "MyPackage")
      app_name = self.class.to_s
      @_anlapp_analysis_chain.make_script(output: output,
                                          package: package,
                                          namespace: namespace,
                                          app_name: app_name)
    end
  end


  # Class of an Analysis chain.
  #
  class AnalysisChain
    # Initialization method.
    #
    def initialize()
      @console = true
      @num_parallels = 1
      @display_frequency = nil
      @parameters_json_filename = nil
      @parameters_json_master = true
      @module_list = []
      @module_hash = {}
      @current_module = nil
      @set_param_list = []
      @namespace_list = [Object]
      @modification_block = nil
      @stage = nil
    end

    # Accessors to internal information (instance variables).
    attr_accessor :console
    attr_accessor :num_parallels
    attr_accessor :current_module
    attr_accessor :display_frequency
    attr_accessor :parameters_json_filename
    attr_accessor :parameters_json_master

    # Clear all internal information on the analysis chain.
    # But it keeps setting not strongly related to the chain.
    # (e.g., console, parallel number, display frequency)
    #
    def clear()
      @module_list.clear
      @module_hash = {}
      @current_module = nil
      @set_param_list.clear
      @namespace_list.clear; @namespace_list << Object
      @modification_block = nil
      @stage = nil
    end

    def definition_already_done?()
      @stage != nil
    end

    def finalization_done?()
      @stage == :finalization_done
    end

    def chain_empty?()
      @module_list.empty?
    end

    # Add namespace (module) into search list of ANL modules.
    #
    # @param [Module] ns namespace to be added to the search list.
    #
    def add_namespace(ns)
      @namespace_list << ns
    end

    # Push an ANL module to the module chain.
    #
    # @param [ANLModule] anl_module ANL module to be pushed.
    # @return [ANLModule] ANL module pushed.
    #
    def push(anl_module)
      module_id = anl_module.module_id.to_sym
      if @module_hash.has_key? module_id
        raise "ANL module #{module_id} is already registered."
      end
      @module_hash[module_id] = anl_module
      @module_list << anl_module
      @current_module = anl_module
    end

    # Insert an ANL module to the module chain at a specified position.
    #
    # @param [Integer] index position to be inserted.
    # @param [ANLModule] anl_module ANL module to be inserted.
    # @return [ANLModule] ANL module inserted.
    #
    def insert(index, anl_module)
      module_id = anl_module.module_id.to_sym
      if @module_hash.has_key? module_id
        raise "ANL module #{module_id} is already registered."
      end
      @module_hash[module_id] = anl_module
      @module_list.insert(index, anl_module)
      @current_module = anl_module
    end

    # Insert an ANL module to the module chain just before a specified module.
    #
    # @param [ANLModule] anl_module ANL module to be inserted.
    # @param [Symbol] module_id_target target module
    # @return [ANLModule] ANL module inserted.
    #
    def insert_before(anl_module, module_id_target)
      insert(index(module_id_target.to_sym), anl_module)
    end

    # Insert an ANL module to the module chain just after a specified module.
    #
    # @param [ANLModule] anl_module ANL module to be inserted.
    # @param [Symbol] module_id_target target module
    # @return [ANLModule] ANL module inserted.
    #
    def insert_after(anl_module, module_id_target)
      insert(index(module_id_target.to_sym)+1, anl_module)
    end

    # Push an ANL module that is specified by a symbol to the module chain.
    #
    # @param [Symbol, Class] anl_module_class ANL module to be pushed.
    # @param [String] module_id ANL module ID.
    # @return [ANLModule] ANL module pushed.
    #
    def chain(anl_module_class, module_id=nil)
      if anl_module_class.is_a?(String) || anl_module_class.is_a?(Symbol)
        class_found = false
        @namespace_list.each do |ns|
          if ns.const_defined? anl_module_class
            anl_module_class = ns.const_get(anl_module_class)
            class_found = true
            break
          end
        end
        class_found or raise "Not found class name #{anl_module_class}."
      end
      mod = anl_module_class.new
      mod.set_module_id(module_id.to_s) if module_id
      push(mod)
    end

    # Get an ANL module by module symbol and set it to the current module.
    #
    # @param [Symbol] module_id ANL module ID.
    # @return [ANLModule] ANL module.
    #
    def expose_module(module_id)
      mod = get_module(module_id)
      if mod
        @current_module = mod
      else
        raise "ANL module #{module_id} is not registered."
      end
      mod
    end

    # Get an ANL module by module symbol.
    # This method does not change the current module.
    #
    # @param [Symbol] module_id ANL module ID.
    # @return [ANLModule] ANL module.
    #
    def get_module(module_id)
      @module_hash[module_id]
    end

    def get_parallel_module(chain_id, module_id)
      @anl.access_to_module(chain_id, module_id.to_s)
    end

    # Get position of the first ANL module which has the specified name
    # in the analysis chain.
    #
    # @param [Symbol] module_id ANL module ID.
    # @return [Integer] Position.
    #
    def index(module_id)
      @module_list.index{|mod| mod.module_id.to_sym==module_id }
    end

    # Get position of the last ANL module which has the specified name
    # in the analysis chain.
    #
    # @param [Symbol] module_id ANL module ID.
    # @return [Integer] Position.
    #
    def rindex(module_id)
      @module_list.rindex{|mod| mod.module_id.to_sym==module_id }
    end

    # Set a text describing the current module.
    #
    # @param [String] description a text description for the current module.
    #
    def text(description)
      @current_module.set_module_description(description)
      return
    end

    # Set a parameter of the current module.
    # Before the ANL definition stage, this method reserves parameter setting,
    # and acctual setting to the ANL module object is performed after the
    # definition stage.
    # If this method is called after the definition stage, parameter setting is
    # performed immediately.
    #
    # @param [String] name name of the parameter.
    # @param value value to be set.
    #
    def set_parameter(name, value)
      mod = @current_module

      if value.is_a? Hash
        value.each do |k, vs|
          vs = [vs] unless vs.is_a? Array
          h = {}
          vs.each_with_index do |v, i|
            h[i] = v
          end
          insert_to_map(name, k, h)
        end
        return
      end

      set_param =
        if value.is_a? Vector
          if value.z
            lambda{ mod.set_parameter(name, value.x, value.y, value.z) }
          else
            lambda{ mod.set_parameter(name, value.x, value.y) }
          end
        elsif value.is_a? Array
          if value.empty?
            lambda{ mod.clear_array(name) }
          else
            f = value.first
            if f.is_a? String
              lambda{ mod.set_parameter_vector_str(name, value) }
            elsif f.is_a? Float
              lambda{ mod.set_parameter_vector_d(name, value) }
            elsif f.is_a? Integer
              lambda{ mod.set_parameter_vector_i(name, value) }
            else
              raise "AnalysisChain#set_parameter(): type invalid (Array of *)."
            end
          end
        else
          if value.is_a? Integer
            lambda{ mod.set_parameter_integer(name, value) }
          else
            lambda{ mod.set_parameter(name, value) }
          end
        end

      set_param_or_raise = lambda do
        begin
          set_param.call
        rescue
          puts "AnalysisChain#set_parameter(): exception detected ===> #{name} in module #{mod.module_id}"
          raise
        end
      end

      if self.definition_already_done?
        set_param_or_raise.call
      else
        @set_param_list << set_param_or_raise
      end

      return
    end

    # Set a map-type parameter of the current module.
    # Before the ANL definition stage, this method reserves parameter setting,
    # and acctual setting to the ANL module object is performed after the
    # definition stage.
    # If this method is called after the definition stage, parameter setting is
    # performed immediately.
    #
    # @param [String] map_name name of the parameter.
    # @param [String] key key of the map content, or name of inserted object.
    # @param [Hash] map_values list of (key, value) pairs of the parameter.
    #
    def insert_to_map(map_name, key, map_values)
      mod = @current_module
      if mod.is_a? ModuleInitializer
        mod.insert_to_map(map_name, key, map_values)
        return
      end

      set_param = lambda do
        mod.insert_to_map(map_name.to_s, key.to_s) do |v|
          map_values.each do |value_name, value|
            if value_name.is_a? Integer
              value_name = v.get_parameter(map_name).value_element_name(value_name)
            end
            v.set_value_element(value_name.to_s, value)
          end
        end
      end

      set_param_or_raise = lambda do
        begin
          set_param.call
        rescue
          puts "AnalysisChain#insert_to_map(): exception detected ===> #{map_name}/#{key} in module #{mod.module_id}"
          raise
        end
      end

      if self.definition_already_done?
        set_param_or_raise.call
      else
        @set_param_list << set_param_or_raise
      end

      return
    end

    # Set a vector-type parameter of the current module.
    # Before the ANL definition stage, this method reserves parameter setting,
    # and acctual setting to the ANL module object is performed after the
    # definition stage.
    # If this method is called after the definition stage, parameter setting is
    # performed immediately.
    #
    # @param [String] vector_name name of the parameter.
    # @param [Hash] values list of (key, value) pairs of the parameter.
    #
    def push_to_vector(vector_name, values)
      mod = @current_module
      if mod.is_a? ModuleInitializer
        mod.push_to_vector(vector_name, values)
        return
      end

      set_param = lambda do
        mod.push_to_vector(vector_name.to_s) do |v|
          values.each do |value_name, value|
            if value_name.is_a? Integer
              value_name = v.get_parameter(map_name).value_element_name(value_name)
            end
            v.set_value_element(value_name.to_s, value)
          end
        end
      end

      set_param_or_raise = lambda do
        begin
          set_param.call
        rescue
          puts "AnalysisChain#insert_to_map(): exception detected ===> #{vector_name} in module #{mod.module_id}"
          raise
        end
      end

      if self.definition_already_done?
        set_param_or_raise.call
      else
        @set_param_list << set_param_or_raise
      end

      return
    end

    # Utility method to set parameters of the specified module.
    # Before the ANL definition stage, this method reserves parameter setting,
    # and acctual setting to the ANL module object is performed after the
    # definition stage.
    # If this method is called after the definition stage, parameter setting is
    # performed immediately.
    #
    # @param [Symbol] module_id ANL module ID.
    # @param [Hash] parameters list of parameters (name, value).
    # @yield [mod] a block can be given for additional process.
    # @yieldparam mod the current module is given.
    #
    def set_parameters(module_id, parameters=nil, &set_param)
      expose_module(module_id)
      with_parameters(parameters, &set_param)
    end

    # Utility method to set parameters of the current module.
    # Before the ANL definition stage, this method reserves parameter setting,
    # and acctual setting to the ANL module object is performed after the
    # definition stage.
    # If this method is called after the definition stage, parameter setting is
    # performed immediately.
    #
    # @param [Hash] parameters list of parameters (name, value).
    # @yield [mod] a block can be given for additional process.
    # @yieldparam mod the current module is given.
    #
    def with_parameters(parameters=nil, &set_param)
      mod = @current_module
      if mod.is_a? ModuleInitializer
        mod.with_parameters(parameters, &set_param)
        return
      end

      if parameters
        parameters.each{|name, value| set_parameter(name.to_s, value) }
      end
      if block_given?
        if self.definition_already_done?
          set_param.(mod)
        else
          @set_param_list << lambda{ set_param.(mod) }
        end
      end
      return
    end

    alias :with :with_parameters

    # Utility method to push ANL module and then set parameters.
    #
    def chain_with_parameters(initializer)
      if initializer
        if initializer.respond_to? :each
          initializer.each{|mi| chain_with_parameters(mi) }
        else
          chain(initializer.module_class, initializer.module_id)
          initializer.play_methods(self)
        end
      end
    end

    # Define a block that will be executed between pre-initialization
    # and initialization.
    #
    def modify(&b)
      @modification_block = b
    end

    # Utility method to modify parameters of the specified module.
    # This method is supposed to be called in the block given to
    # modify method. This block is executed just after pre-initialization.
    #
    def modify_parameters(chain_id, module_id, parameters=nil, &set_param)
      begin
        if chain_id == 0
          expose_module(module_id)
          with_parameters(parameters, &set_param)
        else
          @current_module = get_parallel_module(chain_id, module_id)
          with_parameters(parameters, &set_param)
        end
      ensure
        @current_module = nil
      end
    end

    # Proposed display frequency based on the number of loops.
    # This method is private.
    #
    # @param [Integer] num_loop number of loops. -1 for infinite loops.
    # @return [Integer] frequency of displaying loop ID to STDOUT.
    #
    def proposed_display_frequency(num_loop)
      if num_loop < 0
        display_frequency = 10000
      elsif num_loop < 100
        display_frequency = 1
      else
        display_frequency = 10**((Math.log10(num_loop)-1.5).to_i)
      end
      return display_frequency
    end
    private :proposed_display_frequency

    # Check ANL status if we can go ahead.
    # If it is OK, return true,
    # if it is an error, raise an exception,
    # otherwise false.
    # This method is private.
    #
    # param[Integer] status ANL status.
    # param[String] function name of ANL routine.
    #
    def check_status(status, function_name)
      if status == AS_OK
        return true
      end
      if status == AS_SKIP_ERROR || status == AS_QUIT_ERROR || status == AS_QUIT_ALL_ERROR
        raise "#{function_name} returned #{ANL.show_status(status)}."
      end
      return false
    end
    private :check_status

    # Execute the ANL definition stage.
    #
    def define()
      if @num_parallels > 1
        @anl = ANL::ANLManagerMT.new(@num_parallels)
      else
        @anl = ANL::ANLManager.new
      end

      vec = ANL::ModuleVector.new(@module_list)
      @anl.set_modules(vec)

      status = @anl.Define()
      check_status(status, "Define()") or return
      @stage = :definition_done

      return @anl
    end

    def load_all_parameters()
      if not self.definition_already_done?
        raise "Definition stage is not completed."
      end

      while s = @set_param_list.shift
        s.call
      end
      @stage = :loading_parameters_done
    end

    # Run the ANL analysis.
    #
    # @param [Integer] num_loop number of loops. :all or -1 for infinite loops.
    #
    def run(num_loop)
      if num_loop == :all; num_loop = -1; end
      @display_frequency ||= proposed_display_frequency(num_loop)

      anl = define() unless self.definition_already_done?

      load_all_parameters() unless @stage == :loading_parameters_done

      if parameters_json_filename() && parameters_json_master()
        File.open(parameters_json_filename(), 'w') do |fout|
          fout.print(JSON.pretty_generate(parameters_to_object()))
        end
      end

      status = anl.PreInitialize()
      check_status(status, "PreInitialize()") or return
      @stage = :pre_initialization_done

      if @modification_block
        @modification_block.(self)
      end

      if parameters_json_filename() && !parameters_json_master()
        anl.parameters_to_json(parameters_json_filename())
      end

      status = anl.Initialize()
      check_status(status, "Initialize()") or return
      @stage = :initialization_done

      puts ""
      puts "<Begin Analysis> | Time: " + Time.now.to_s
      $stdout.flush
      anl.set_display_frequency(@display_frequency)
      status = anl.Analyze(num_loop, @console)
      puts ""
      puts "<End Analysis>   | Time: " + Time.now.to_s
      $stdout.flush
      check_status(status, "Analyze()") or return
      @stage = :analysis_done

      status = anl.Finalize()
      check_status(status, "Finalize()") or return
      @stage = :finalization_done
    rescue RuntimeError => ex
      puts ""
      puts "################################################################"
      puts "#                                                              #"
      puts "#                       ANL Exception                          #"
      puts "#                                                              #"
      puts "################################################################"
      puts ""
      puts ex
      puts ""
      puts "################################################################"
      puts ""
      puts "---- displayed by <Ruby> AnalysisChain#run() ----"
      puts ""

      if ANL::ANLException.VerboseLevel >= 3
        raise
      end
    end

    # Start the ANL interactive session for running the ANL analysis.
    #
    def run_interactive()
      anl = define() unless self.definition_already_done?

      load_all_parameters() unless @stage == :loading_parameters_done

      status = anl.do_interactive_comunication()
      check_status(status, "do_interactive_comunication()") or return

      if parameters_json_filename() && parameters_json_master()
        File.open(parameters_json_filename(), 'w') do |fout|
          fout.print(JSON.pretty_generate(parameters_to_object()))
        end
      end

      status = anl.PreInitialize()
      check_status(status, "PreInitialize()") or return
      @stage = :pre_initialization_done

      if parameters_json_filename() && !parameters_json_master()
        anl.parameters_to_json(parameters_json_filename())
      end

      status = anl.Initialize()
      check_status(status, "Initialize()") or return
      @stage = :initialization_done

      status = anl.do_interactive_analysis()
      check_status(status, "do_interactive_analysis()") or return
      @stage = :analysis_done

      status = anl.Finalize()
      check_status(status, "Finalize()") or return
      @stage = :finalization_done
    rescue RuntimeError => ex
      puts ""
      puts "################################################################"
      puts "#                                                              #"
      puts "#                       ANL Exception                          #"
      puts "#                                                              #"
      puts "################################################################"
      puts ""
      puts ex
      puts ""
      puts "################################################################"
      puts ""
      puts "---- displayed by <Ruby> AnalysisChain#run_interactive() ----"
      puts ""

      if ANL::ANLException.VerboseLevel >= 3
        raise
      end
    end

    # Print all parameters of all the module in the analysis chain.
    #
    def print_all_parameters()
      @module_list.each{|m|
        puts "--- "+m.module_id+" ---"
        m.print_parameters
        puts ''
      }
    end

    def parameters_to_object()
      { :application => { :module_list => @module_list.map(&:parameters_to_object) } }
    end

    # Make a XML document describing module parameters.
    #
    # @param [STRING] output output XML filename. If nil, output to STDOUT.
    # @param [STRING] namespace
    #
    def make_doc(output: nil, namespace: "ANL")
      doc = REXML::Document.new
      doc << REXML::XMLDecl.new('1.0', 'UTF-8')
      node = doc.add_element("anlmodules") #, {"xmlns" => "http://www.w3.org/2010/ANLNext"})
      node.add_element("namespace").add_text namespace
      @module_list.each do |mod|
        o = node.add_element("module")
        o.add_element("name").add_text mod.module_name
        o.add_element("version").add_text mod.module_version
        o.add_element("text").add_text mod.module_description
        ps = o.add_element("parameters")
        mod.parameter_list.each do |param|
          type = param.type_name
          container_type = ""
          if type=="map" || type=="vector"
            container_type = type.to_sym
          end

          p = ps.add_element("param", {"container_type" => container_type})
          p.add_element("name").add_text param.name
          p.add_element("type").add_text type
          p.add_element("unit").add_text param.unit_name
          if type=="vector<string>" || type=="list<string>"
            p.add_element("default_value").add_text param.default_string
          else
            p.add_element("default_value").add_text param.value_string
          end
          p.add_element("description").add_text param.description

          if container_type == :map
            p = ps.add_element("param", {"container_type" => "key"})
            p.add_element("name").add_text param.map_key_name
            p.add_element("type").add_text "string"
            p.add_element("unit").add_text ""
            p.add_element("default_value").add_text param.default_string
            p.add_element("description").add_text ""

            param.num_value_elements.times do |i|
              value = param.value_element_info(i)
              p = ps.add_element("param", {"container_type" => "value"})
              p.add_element("name").add_text value.name
              p.add_element("type").add_text value.type_name
              p.add_element("unit").add_text value.unit_name
              p.add_element("default_value").add_text value.value_string
              p.add_element("description").add_text value.description
            end
          elsif container_type == :vector
            param.num_value_elements.times do |i|
              value = param.value_element_info(i)
              p = ps.add_element("param", {"container_type" => "value"})
              p.add_element("name").add_text value.name
              p.add_element("type").add_text value.type_name
              p.add_element("unit").add_text value.unit_name
              p.add_element("default_value").add_text value.value_string
              p.add_element("description").add_text value.description
            end
          end
        end
      end

      formatter = REXML::Formatters::Pretty.new
      formatter.compact = true
      if output
        File::open(output, 'w') do |fo|
          formatter.write(doc, fo)
          fo.puts ''
        end
      else
        formatter.write(doc, STDOUT)
        puts ''
      end
    end

    # Make a Ruby run script template for this ANL chain.
    #
    # @param [STRING] output output script filename. If nil, output to STDOUT.
    # @param [STRING] package name of the Ruby extension library.
    # @param [STRING] namespace namespace of the Ruby extension module.
    # @param [STRING] app_name class name of the application
    #
    def make_script(output: nil,
                    package: "myPackage",
                    namespace: "MyPackage",
                    app_name: "MyApp")
      out = output ? File::open(output, 'w') : STDOUT
      out.puts '#!/usr/bin/env ruby'
      out.puts "require 'ANLLib'"
      out.puts "require '#{package}'"
      out.puts ''
      out.puts 'num_loop = 100000'
      out.puts ''
      out.puts "class #{app_name} < ANL::ANLApp"
      out.puts "  def setup()"
      out.puts "    add_namespace #{namespace}"
      out.puts ""
      @module_list.each do |mod|
        out.puts '    chain :'+mod.module_name
        with_parameters_string = '    with_parameters('+ mod.parameter_list.select{|param|
          param.type_name != 'map' && param.type_name != 'vector'
        }.map do |param|
          type = param.type_name
          default_string =
            case type
            when 'vector<string>', 'list<string>'
              '["'+param.default_string+'"]'
            when 'vector<int>', 'vector<double>'
              '[]'
            when 'string'
              '"'+param.value_string+'"'
            when 'double'
              param.value_string.to_f.to_s
            when 'bool'
              {'1'=>'true', '0'=>'false'}[param.value_string]
            when '2-vector'
              x, y = param.value_string.strip.split(/\s+/)
              'vec('+x.to_f.to_s+', '+y.to_f.to_s+')'
            when '3-vector'
              x, y, z = param.value_string.strip.split(/\s+/)
              'vec('+x.to_f.to_s+', '+y.to_f.to_s+', '+z.to_f.to_s+')'
            else
              param.value_string
            end
          "\"#{param.name}\" => #{default_string}"
        end.join(",\n                    ") + ")"
        out.puts with_parameters_string

        mod.parameter_list.select{|param|
          param.type_name == 'map'
        }.each do |param|
          default_string = param.default_string
          out.puts '    insert_to_map "'+param.name+'", "'+default_string+'", {'
          param.num_value_elements.times do |i|
            mapValue = param.value_element_info(i)
            type = mapValue.type_name
            default_string =
              case type
              when 'string'
                '"'+mapValue.value_string+'"'
              when 'double'
                mapValue.value_string.to_f.to_s
              else
                mapValue.value_string
              end
            out.puts '      "'+mapValue.name+'" => '+default_string+","
          end
          out.puts '    }'
        end

        mod.parameter_list.select{|param|
          param.type_name == 'vector'
        }.each do |param|
          default_string = param.default_string
          out.puts '    push_to_vector "'+param.name+'", {'
          param.num_value_elements.times do |i|
            mapValue = param.value_element_info(i)
            type = mapValue.type_name
            default_string =
              case type
              when 'string'
                '"'+mapValue.value_string+'"'
              when 'double'
                mapValue.value_string.to_f.to_s
              else
                mapValue.value_string
              end
            out.puts '      "'+mapValue.name+'" => '+default_string+","
          end
          out.puts '    }'
        end

        out.puts ''
      end

      out.puts "  end"
      out.puts "end"
      out.puts ""
      out.puts "anl = #{app_name}.new"
      out.puts "anl.run(num_loop)"
      out.close if output
    end
  end


  # Class of a SWIG Interface to ANL module class.
  # This produces one Ruby class interface to be processed by SWIG.
  #
  class SWIGClass
    def initialize(name='', manual=false, option=nil)
      @name = name
      @manual = manual
      @option = option
      @include_path = ['../include']
    end

    attr_reader :name, :manual, :option
    attr_writer :include_path

    def puts_include()
      puts "#ifdef "+@option if @option
      puts '#include "'+@name+'.hh"'
      puts "#endif" if @option
    end

    def interface()
      puts "#ifdef "+@option if @option
      if @manual
        File::open(@name+".i") {|fin|
          print fin.read
        }
        puts ''
      else
        extract_interface
      end
      puts "#endif" if @option
      puts ""
    end

    def extract_interface()
      c = @name
      dirIndex = @include_path.index do |dir|
        File::exist?(dir+"/"+c+".hh")
      end
      fileName = @include_path[dirIndex]+"/"+c+".hh"
      File.open(fileName) do |fin|
        className = nil
        classOpen = nil
        constructor = nil
        constructor2 = nil
        numNestedClasses = 0

        fin.each_line do |l|
          if !className && l=~/^class\s+(\w+)(\s+|\:)/
            className = $1
            puts l.sub(/,.+/, '')
            if l.include? '{'
              classOpen = true
            end
            next
          end

          if className
            if !classOpen
              puts l
              if l.include? '{'
                classOpen = true
                puts 'public:'
              end
            else
              if l.include?("class")
                if l.include? '};'
                  # nested class closed
                else
                  numNestedClasses += 1
                end
                next
              end

              if numNestedClasses > 0
                if l.include? '};'
                  # nested class closed
                  numNestedClasses -= 1
                end
                next
              end

              if l.include?(className+'(') && !l.include?('&')
                puts l
                constructor = true
              elsif constructor
                puts l
              end

              if constructor
                constructor = nil if !constructor2 && l.include?(';')
                constructor2 = true if l.include?('{')
                if constructor2 && l.include?('}')
                  constructor = nil
                  constructor2 = nil
                end
              end

              if l.include?('};')
                puts l
                puts ''
                classOpen = nil
                className = nil
              end
            end
          end
        end
      end
    end
  end


  # Class of a SWIG Interface to a list of ANL modules.
  # This produces one Ruby extension module interface to be processed by SWIG.
  #
  class SWIGModule
    def initialize(name, classList, namespace=nil)
      @name = name
      @classList = classList
      @namespace = namespace
      @includeFiles = []
      @importModules = []
      @includeModules = []
    end

    attr_reader :name
    attr_accessor :includeFiles, :importModules, :includeModules

    def print_interface(using_namespace=true)
      puts '%module '+@name
      puts '%{'
      @classList.each{|sc| sc.puts_include }
      puts ""
      @includeFiles.each{|s| puts '#include "'+s+'"' }
      puts ''
      puts '%}'
      puts ''
      @includeModules.each{|s| puts '%include "'+s+'"' }
      puts ''
      @importModules.each{|s| puts '%import "'+s+'"' }
      puts ''
      puts 'namespace '+@namespace+' {' if @namespace
      puts ''
      @classList.each{|sc| sc.interface }
      puts '}' if @namespace
    end

    def print_class_list()
      puts 'namespace '+@namespace+' {' if @namespace
      @classList.each{|s| puts 'class '+s.name+';' }
      puts '}' if @namespace
    end

    def print_make_doc()
      @classList.each do |s|
        module_name = @name[0].upcase+@name[1..-1]
        puts "a.push #{module_name}::#{s.name}.new"
        puts "# a.text \"\""
        puts ""
      end
    end
  end


  # ParallelRun utility
  #
  begin
    require 'parallel'

    class ParallelRun
      include Parallel

      def initialize()
        @num_processes = Parallel.processor_count
        @make_log_name = nil
        @log_name = nil
      end

      attr_accessor :num_processes

      def set_log(filename=nil, &block)
        if filename==nil && block_given?
          @make_log_name = block
        else
          @log_name = filename
        end
      end

      def run1(list)
        yield list.shift
      end

      def run(list, testrun: false)
        if testrun
          yield list[0]
          return
        end

        until list.empty?
          Parallel.map(list.shift(@num_processes),
                       :in_processes => @num_processes) do |run|
            if @log_name
              log_file = @log_name % run
            else
              log_file = @make_log_name.(run)
            end
            File.open(log_file, 'w') do |fo|
              STDOUT.reopen(fo); STDOUT.sync = true
              STDERR.reopen(fo); STDERR.sync = true
              yield run
            end
          end
        end
      end
    end
  rescue LoadError
    # do nothing
  end


  # MPIRun utility
  #
  begin
    require 'mpi'

    class MPIRun
      def initialize()
        @make_log_name = nil
        @log_name = nil
      end

      def set_log(filename=nil, &block)
        if filename==nil && block_given?
          @make_log_name = block
        else
          @log_name = filename
        end
      end

      def run1()
        rank = 0
        yield rank
      end

      def run()
        begin
          MPI.Init
          world = MPI::Comm::WORLD
          rank = world.rank

          if @log_name
            log_file = @log_name % rank
          else
            log_file = @make_log_name.(rank)
          end

          File.open(log_file, 'w') do |fo|
            STDOUT.reopen(fo); STDOUT.sync = true
            STDERR.reopen(fo); STDERR.sync = true
            yield rank
          end
        ensure
          MPI.Finalize
        end
      end
    end
  rescue LoadError
    # do nothing
  end

end # module ANL


# A utility method to make a Vector object.
#
# @param [Float] x first component of the vector.
# @param [Float] y second component of the vector.
# @param [Float] z third component of the vector.
# @return [Vector] a Vector object (x, y, z)
#
def vec(x, y, z=nil)
  return ANL::Vector.new(x, y, z)
end
