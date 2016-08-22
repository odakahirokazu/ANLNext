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

    def to_object()
      type = type_name()
      o = {}
      o[:name] = name()
      o[:type] = type

      case type
      when "double", "vector<double>"
        o[:unit] = unit()
        o[:unit_name] = unit_name()
      end

      case type
      when "vector"
        values = []
        o[:value] = values
        size_of_container.times do |k|
          value = []
          values.push(value)
          retrieve_from_container(k)
          num_value_elements.times do |i|
            element = value_element_info(i)
            value.push(element.to_object)
          end
        end
      when "map"
        values = {}
        o[:value] = values
        map_key_list.each do |k|
          value = []
          values[k] = value
          retrieve_from_container(k)
          num_value_elements.times do |i|
            element = value_element_info(i)
            value.push(element.to_object)
          end
        end
      else
        o[:value] = get_value_auto()
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
      ANL::ANLStatus_AS_OK => "AS_OK",
      ANL::ANLStatus_AS_SKIP => "AS_SKIP",
      ANL::ANLStatus_AS_SKIP_ERR => "AS_SKIP_ERR",
      ANL::ANLStatus_AS_QUIT => "AS_QUIT",
      ANL::ANLStatus_AS_QUIT_ERR => "AS_QUIT_ERR",
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

    def write_parameters_to_json(filename)
      @_anlapp_analysis_chain.parameters_json_filename = filename
    end

    # Run this application.
    #
    # @param [Fixnum] num_loop number of loops. :all or -1 for infinite loops.
    # @param [Fixnum] display_frequency frequency of displaying loop ID
    #     to STDOUT.
    #
    def run(num_loop, display_frequency=nil)
      if @_anlapp_analysis_chain.analysis_done?
        @_anlapp_analysis_chain.clear()
      end
      setup()
      @_anlapp_analysis_chain.run(num_loop, display_frequency)
    end

    ### method delegation to the AnalysisChain object.
    anlapp_methods = [
      :thread_mode, :thread_mode=, :add_namespace,
      :push, :insert, :insert_before, :insert_after, :chain,
      :expose_module, :get_module, :index, :rindex,
      :insert_to_map, :push_to_vector,
      :set_parameters, :with_parameters, :chain_with_parameters,
      :startup, :prepare_all_parameters,
      :print_all_parameters, :parameters_to_object, :make_doc,
    ]
    def_delegators :@_anlapp_analysis_chain, *anlapp_methods
    alias :with :with_parameters

    def self.define_setup_module(basename, module_class=nil,
                                 array: false, take_parameters: false)
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

      if module_class
        if take_parameters
          define_method(setter_name) do |parameters=nil, &set_param|
            mi = ModuleInitializer.new(module_class)
            set_function.(self, variable_name, mi, array)
            @_anlapp_analysis_chain.current_module = mi
            mi.with(parameters, &set_param)
            mi
          end
        else
          define_method(setter_name) do |module_id=nil|
            mi = ModuleInitializer.new(module_class, module_id)
            set_function.(self, variable_name, mi, array)
            @_anlapp_analysis_chain.current_module = mi
            mi
          end
        end
      else
        if take_parameters
          define_method(setter_name) do |mod1, parameters=nil, &set_param|
            mi = ModuleInitializer.new(mod1)
            set_function.(self, variable_name, mi, array)
            @_anlapp_analysis_chain.current_module = mi
            mi.with(parameters, &set_param)
            mi
          end
        else
          define_method(setter_name) do |mod1, module_id=nil|
            mi = ModuleInitializer.new(mod1, module_id)
            set_function.(self, variable_name, mi, array)
            @_anlapp_analysis_chain.current_module = mi
            mi
          end
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
      @module_list = []
      @module_hash = {}
      @current_module = nil
      @set_param_list = []
      @set_module_list = []
      @namespace_list = [Object]
      @startup_done = false
      @analysis_done = false
      @thread_mode = true
      @parameters_json_filename = nil
    end

    # Accessors to internal information (instance variables).
    attr_accessor :thread_mode
    attr_accessor :current_module
    attr_accessor :parameters_json_filename
    def startup_done?(); @startup_done; end
    def analysis_done?(); @analysis_done; end

    # Clear all internal information including the analysis chain.
    # But it keeps thread_mode setting.
    #
    def clear()
      @module_list.clear
      @module_hash = {}
      @current_module = nil
      @set_param_list.clear
      @set_module_list.clear
      @namespace_list.clear; @namespace_list << Object
      @startup_done = false
      @analysis_done = false
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
    # @param [Fixnum] index position to be inserted.
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
      if anl_module_class.class==String || anl_module_class.class==Symbol
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

    # Get position of the first ANL module which has the specified name
    # in the analysis chain.
    #
    # @param [Symbol] module_id ANL module ID.
    # @return [Fixnum] Position.
    #
    def index(module_id)
      @module_list.index{|mod| mod.module_id.to_sym==module_id }
    end

    # Get position of the last ANL module which has the specified name
    # in the analysis chain.
    #
    # @param [Symbol] module_id ANL module ID.
    # @return [Fixnum] Position.
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
    # Before the ANL startup session, this method reserves parameter setting,
    # and acctual setting to the ANL module object is performed after the
    # startup session.
    # If this method is called after the startup session, parameter setting is
    # performed immediately.
    #
    # @param [String] name name of the parameter.
    # @param value value to be set.
    #
    def setp(name, value)
      mod = @current_module

      if value.class == Hash
        value.each do |k, vs|
          vs = [vs] unless vs.class == Array
          h = {}
          vs.each_with_index do |v, i|
            h[i] = v
          end
          insert_to_map(name, k, h)
        end
        return
      end

      set_param =
        if value.class == Vector
          if value.z
            lambda{ mod.set_parameter(name, value.x, value.y, value.z) }
          else
            lambda{ mod.set_parameter(name, value.x, value.y) }
          end
        elsif value.class == Array
          if value.empty?
            lambda{ mod.clear_array(name) }
          else
            f = value.first
            if f.class == String
              lambda{ mod.set_parameter_vector_str(name, value) }
            elsif f.class == Float
              lambda{ mod.set_parameter_vector_d(name, value) }
            elsif f.integer?
              lambda{ mod.set_parameter_vector_i(name, value) }
            else
              raise "ANLApp#setp(): type invalid (Array of *)."
            end
          end
        else
          lambda{ mod.set_parameter(name, value) }
        end

      set_param_or_raise = lambda do
        begin
          set_param.call
        rescue
          puts "Set parameter exception: #{name} in module #{mod.module_id}"
          raise
        end
      end

      if @startup_done
        set_param_or_raise.call
      else
        @set_param_list << set_param_or_raise
      end

      return
    end

    # Set a map-type parameter of the current module.
    # Before the ANL startup session, this method reserves parameter setting,
    # and acctual setting to the ANL module object is performed after the
    # startup session.
    # If this method is called after the startup session, parameter setting is
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
          puts "Set parameter exception: #{map_name}/#{key} in module #{mod.module_id}"
          raise
        end
      end

      if @startup_done
        set_param_or_raise.call
      else
        @set_param_list << set_param_or_raise
      end

      return
    end

    # Set a vector-type parameter of the current module.
    # Before the ANL startup session, this method reserves parameter setting,
    # and acctual setting to the ANL module object is performed after the
    # startup session.
    # If this method is called after the startup session, parameter setting is
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
          puts "Set parameter exception: #{vector_name} in module #{mod.module_id}"
          raise
        end
      end

      if @startup_done
        set_param_or_raise.call
      else
        @set_param_list << set_param_or_raise
      end

      return
    end

    # Utility method to set parameters of the specified module.
    # Before the ANL startup session, this method reserves parameter setting,
    # and acctual setting to the ANL module object is performed after the
    # startup session.
    # If this method is called after the startup session, parameter setting is
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
    # Before the ANL startup session, this method reserves parameter setting,
    # and acctual setting to the ANL module object is performed after the
    # startup session.
    # If this method is called after the startup session, parameter setting is
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

      unless @set_module_list.include? mod
        @set_module_list << mod
      end
      if parameters
        parameters.each{|name, value| setp(name.to_s, value) }
      end
      if block_given?
        set_param.(mod)
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

    # Proposed display frequency based on the number of loops.
    # This method is private.
    #
    # @param [Fixnum] num_loop number of loops. -1 for infinite loops.
    # @return [Fixnum] frequency of displaying loop ID to STDOUT.
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

    # Check ANL status. If it is not OK, raise an exception.
    # This method is private.
    #
    # param[Fixnum] status ANL status.
    # param[String] function name of ANL routine.
    #
    def check_status(status, function_name)
      unless status == ANL::ANLStatus_AS_OK
        raise "#{function_name} returned #{ANL.show_status(status)}."
      end
      true
    end
    private :check_status

    # Execute the ANL startup session.
    #
    def startup()
      @anl = ANL::ANLManager.new
      vec = ANL::ModuleVector.new(@module_list)
      @anl.SetModules(vec)
      status = @anl.Startup()
      check_status(status, "Startup()")
      @startup_done = true
      return @anl
    end

    def prepare_all_parameters()
      while s = @set_param_list.shift
        s.call
      end
    end

    # Run the ANL analysis.
    #
    # @param [Fixnum] num_loop number of loops. :all or -1 for infinite loops.
    # @param [Fixnum] display_frequency frequency of displaying loop ID
    #     to STDOUT.
    #
    def run(num_loop, display_frequency=nil)
      if num_loop == :all; num_loop = -1; end
      display_frequency ||= proposed_display_frequency(num_loop)

      anl = startup()
      prepare_all_parameters()
      if parameters_json_filename()
        File.open(parameters_json_filename(), 'w') do |fout|
          fout.print(JSON.pretty_generate(parameters_to_object()))
        end
      end

      status = anl.Prepare()
      check_status(status, "Prepare()")

      status = anl.Initialize()
      check_status(status, "Initialize()")

      puts ""
      puts "Analysis Begin  | Time: " + Time.now.to_s
      $stdout.flush
      anl.SetDisplayFrequency(display_frequency)
      status = anl.Analyze(num_loop, @thread_mode)
      check_status(status, "Analyze()")
      puts ""
      puts "Analysis End    | Time: " + Time.now.to_s
      $stdout.flush

      status = anl.Exit()
      check_status(status, "Exit()")
      @analysis_done = true
    rescue RuntimeError => ex
      puts ""
      puts "################################################################"
      puts "#                                                              #"
      puts "#                       ANL EXCEPTION                          #"
      puts "#                                                              #"
      puts "################################################################"
      puts ""
      puts ex
      puts ""
      puts "################################################################"
      puts ""
      puts ""

      if ANL::ANLException.VerboseLevel >= 3
        raise
      end
    end

    # Start the ANL interactive session for running the ANL analysis.
    #
    def run_interactive()
      anl = startup()

      prepare_all_parameters()
      @set_module_list.each{|mod|
        status = mod.mod_prepare()
        check_status(status, "#{mod.module_id}::mod_prepare()")
      }

      status = anl.InteractiveCom()
      check_status(status, "InteractiveCom()")

      status = anl.InteractiveAna()
      check_status(status, "InteractiveAna()")

      status = anl.Exit()
      check_status(status, "Exit()")
    rescue RuntimeError => ex
      puts ""
      puts "  ### ANL NEXT Exception ###  "
      puts ""
      puts ex
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
      out.puts 'display_frequency = 1000'
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
      out.puts "anl.run(num_loop, display_frequency)"
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
              if l.include?(className+'(')
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
