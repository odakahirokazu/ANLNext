# anlnext/chain.rb
# Class library written in Ruby for the ANL Next framework.
#
# @author: Hirokazu Odaka
#

require 'forwardable'
require 'rexml/document'
require 'json'

module ANL

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
    # @param [Integer] display_period period of displaying loop ID
    #     to STDOUT.
    #
    def run(num_loop, display_period=nil)
      if @_anlapp_analysis_chain.finalization_done?
        @_anlapp_analysis_chain.clear()
      end

      if @_anlapp_analysis_chain.chain_empty?
        setup()
      end

      if display_period
        @_anlapp_analysis_chain.display_period = display_period
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
      :expose_module, :get_module, :get_parallel_module, :index, :rindex,
      :insert_to_map, :push_to_vector,
      :set_parameter, :set_parameters, :set_module_parameter, :set_module_parameters,
      :with_submodule, :chain_with_parameters,
      :load_parameters_from_yaml,
      :modify, :modify_parameters,
      :define, :load_all_parameters,
      :print_all_parameters, :parameters_to_object, :make_doc,
      :num_parallels, :num_parallels=,
      :display_period=,
    ]
    def_delegators :@_anlapp_analysis_chain, *anlapp_methods
    alias :with_parameters :set_parameters
    alias :with :set_parameters

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
        define_method(setter_name) do |*args, &parameter_setter|
          module_id, parameters = resolve_arguments(args)
          mi = ModuleInitializer.new(module_class, module_id)
          set_function.(self, variable_name, mi, array)
          @_anlapp_analysis_chain.current_module = mi
          mi.with(parameters, &parameter_setter)
          mi
        end
      else
        define_method(setter_name) do |mod1, *args, &parameter_setter|
          module_id, parameters = resolve_arguments(args)
          mi = ModuleInitializer.new(mod1, module_id)
          set_function.(self, variable_name, mi, array)
          @_anlapp_analysis_chain.current_module = mi
          mi.with(parameters, &parameter_setter)
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
      @display_period = nil
      @parameters_json_filename = nil
      @parameters_json_master = true
      @module_list = []
      @module_hash = {}
      @current_module = nil
      @parameter_setter_list = []
      @applying_parameters = false
      @namespace_list = [Object]
      @modification_block = nil
      @stage = nil
      @anl_manager = nil
    end

    # Accessors to internal information (instance variables).
    attr_accessor :console
    attr_accessor :num_parallels
    attr_accessor :current_module
    attr_accessor :display_period
    attr_accessor :parameters_json_filename
    attr_accessor :parameters_json_master

    # Clear all internal information on the analysis chain.
    # But it keeps setting not strongly related to the chain.
    # (e.g., console, parallel number, display period)
    #
    def clear()
      raise 'Cannot clear the chain while applying parameters.' if @applying_parameters

      @module_list.clear
      @module_hash = {}
      @current_module = nil
      @parameter_setter_list.clear
      @namespace_list.clear; @namespace_list << Object
      @modification_block = nil
      @stage = nil
      @anl_manager = nil
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
      @module_hash[module_id.to_sym]
    end

    def get_parallel_module(chain_id, module_id)
      @anl_manager.access_to_module(chain_id, module_id.to_s)
    end

    # Get position of the first ANL module which has the specified name
    # in the analysis chain.
    #
    # @param [Symbol] module_id ANL module ID.
    # @return [Integer] Position.
    #
    def index(module_id)
      @module_list.index{|mod| mod.module_id.to_sym==module_id.to_sym }
    end

    # Get position of the last ANL module which has the specified name
    # in the analysis chain.
    #
    # @param [Symbol] module_id ANL module ID.
    # @return [Integer] Position.
    #
    def rindex(module_id)
      @module_list.rindex{|mod| mod.module_id.to_sym==module_id.to_sym }
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
    # Reserve parameter setting for execution by load_all_parameters().
    #
    # @param [String] name name of the parameter.
    # @param value value to be set.
    #
    def set_parameter(name, value)
      check_parameter_configuration()
      mod = @current_module
      if mod.is_a? ModuleInitializer
        mod.set_parameter(name, value)
        return
      end
      parameter_setter = make_parameter_setter(mod, name, ANL.copy_parameter_value(value))

      reserve_parameter(parameter_setter)

      return
    end

    private def check_parameter_configuration()
      unless @stage.nil? || @stage == :definition_done || @stage == :pre_initialization_done
        raise "Parameter configuration is not allowed at this stage: #{@stage}."
      end
    end

    private def reserve_parameter(parameter_setter)
      check_parameter_configuration()
      raise 'No current module is selected.' unless @current_module

      @parameter_setter_list << parameter_setter
    end

    private def make_parameter_setter(mod, name, value)
      name = name.to_s.dup
      if name.include?('.')
        parts = name.split('.', -1)
        unless parts.size == 2 && parts.none?(&:empty?)
          raise ArgumentError, "Invalid submodule parameter path: #{name}"
        end
        submodule_name, parameter_name = parts
        return lambda do
          submodule = mod.get_submodule(submodule_name)
          make_parameter_setter(submodule, parameter_name, value).call
        end
      end

      if value.is_a? Hash
        lambda do
          if mod.get_parameter(name).type_name() == 'record_map'
            value.each do |key, elements|
              elements = [elements] unless elements.is_a?(Array)
              mod.insert_to_map(name.to_s, key.to_s) do |registry|
                elements.each_with_index do |element, index|
                  element_name = registry.get_parameter(name).value_element_name(index)
                  registry.set_value_element(element_name, element)
                end
              end
            end
          else
            mod.set_parameter(name, value)
          end
        end
      elsif value.is_a? Vector
        if value.z
          lambda{ mod.set_parameter(name, value.x, value.y, value.z) }
        else
          lambda{ mod.set_parameter(name, value.x, value.y) }
        end
      else
        if value.is_a? Integer
          lambda do
            if mod.get_parameter(name).type_name() == 'unsigned_integer'
              mod.set_parameter_unsigned_integer(name, value)
            else
              mod.set_parameter_integer(name, value)
            end
          end
        else
          lambda{ mod.set_parameter(name, value) }
        end
      end
    end

    # Set a map-type parameter of the current module.
    # Reserve map insertion for execution by load_all_parameters().
    #
    # @param [String] map_name name of the parameter.
    # @param [String] key key of the map content, or name of inserted object.
    # @param [Hash] map_values list of (key, value) pairs of the parameter.
    #
    def insert_to_map(map_name, key, map_values)
      check_parameter_configuration()
      mod = @current_module
      if mod.is_a? ModuleInitializer
        mod.insert_to_map(map_name, key, map_values)
        return
      end

      map_name = map_name.to_s.dup
      key = key.to_s.dup
      map_values = ANL.copy_parameter_value(map_values)
      parameter_setter = lambda do
        mod.insert_to_map(map_name.to_s, key.to_s) do |v|
          map_values.each do |value_name, value|
            if value_name.is_a? Integer
              value_name = v.get_parameter(map_name).value_element_name(value_name)
            end
            v.set_value_element(value_name.to_s, value)
          end
        end
      end

      reserve_parameter(parameter_setter)

      return
    end

    # Set a vector-type parameter of the current module.
    # Reserve vector insertion for execution by load_all_parameters().
    #
    # @param [String] vector_name name of the parameter.
    # @param [Hash] values list of (key, value) pairs of the parameter.
    #
    def push_to_vector(vector_name, values)
      check_parameter_configuration()
      mod = @current_module
      if mod.is_a? ModuleInitializer
        mod.push_to_vector(vector_name, values)
        return
      end

      vector_name = vector_name.to_s.dup
      values = ANL.copy_parameter_value(values)
      parameter_setter = lambda do
        mod.push_to_vector(vector_name.to_s) do |v|
          values.each do |value_name, value|
            if value_name.is_a? Integer
              value_name = v.get_parameter(vector_name).value_element_name(value_name)
            end
            v.set_value_element(value_name.to_s, value)
          end
        end
      end

      reserve_parameter(parameter_setter)

      return
    end

    def set_module_parameter(module_id, name, value)
      check_parameter_configuration()
      expose_module(module_id)
      set_parameter(name, value)
    end

    # Utility method to set parameters of the specified module.
    # Settings are reserved for execution by load_all_parameters().
    #
    # @param [Symbol] module_id ANL module ID.
    # @param [Hash] parameters list of parameters (name, value).
    # @yield [mod] a block can be given for additional process.
    # @yieldparam mod the current module is given.
    #
    def set_module_parameters(module_id, parameters=nil, &parameter_setter)
      check_parameter_configuration()
      expose_module(module_id)
      set_parameters(parameters, &parameter_setter)
    end

    def load_parameters_from_yaml(filename)
      check_parameter_configuration()
      require 'yaml'

      text = File.read(filename, encoding: 'UTF-8')
      # safe_load() reads only the first document.
      documents = YAML.parse_stream(text, filename: filename.to_s)
      unless documents.children.size == 1
        raise ArgumentError, 'YAML parameter file must contain exactly one document.'
      end
      parameters_by_module = YAML.safe_load(text, filename: filename.to_s, aliases: true)
      unless parameters_by_module.is_a?(Hash)
        raise ArgumentError, 'YAML parameter file must map module IDs to parameter mappings.'
      end

      settings = parameters_by_module.map do |module_id, parameters|
        unless module_id.is_a?(String) && parameters.is_a?(Hash) && parameters.keys.all?{|name| name.is_a?(String) }
          raise ArgumentError, 'YAML module IDs and parameter names must be strings, with a mapping for each module.'
        end
        mod = get_module(module_id)
        unless mod
          raise ArgumentError, "Unknown module ID in YAML parameter file: #{module_id}"
        end
        [mod, parameters]
      end

      current_module = @current_module
      begin
        settings.each do |mod, parameters|
          @current_module = mod
          set_parameters(parameters)
        end
      ensure
        @current_module = current_module
      end
      return
    end

    # Utility method to set parameters of the current module.
    # Settings and the optional block are reserved for load_all_parameters().
    #
    # @param [Hash] parameters list of parameters (name, value).
    # @yield [mod] a block can be given for additional process.
    # @yieldparam mod the current module is given.
    #
    def set_parameters(parameters=nil, &parameter_setter)
      check_parameter_configuration()
      mod = @current_module
      if mod.is_a? ModuleInitializer
        mod.set_parameters(parameters, &parameter_setter)
        return
      end

      if parameters
        parameters.each do |name, value|
          name = name.to_s
          if name.end_with?('_') && !name.include?('.')
            submodule_name = name[0...-1]
            if submodule_name.empty? || !value.is_a?(Hash)
              raise ArgumentError, "Submodule parameter group requires a nonempty name and a Hash: #{name}"
            end
            value.each do |parameter_name, parameter_value|
              set_parameter("#{submodule_name}.#{parameter_name}", parameter_value)
            end
          else
            set_parameter(name, value)
          end
        end
      end
      if block_given?
        reserve_parameter(lambda{ parameter_setter.(mod) })
      end
      return
    end

    alias :with_parameters :set_parameters
    alias :with :set_parameters

    def with_submodule(name, key)
      name = name.to_s.dup
      key = key.to_s.dup
      set_parameters do |mod|
        mod.set_submodule_by_key(name, key)
      end
    end

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
    def modify_parameters(chain_id, module_id, parameters=nil, &parameter_setter)
      current_module = @current_module
      begin
        if chain_id == 0
          expose_module(module_id)
          set_parameters(parameters, &parameter_setter)
        else
          @current_module = get_parallel_module(chain_id, module_id)
          set_parameters(parameters, &parameter_setter)
        end
      ensure
        @current_module = current_module
      end
    end

    # Proposed display period based on the number of loops.
    # This method is private.
    #
    # @param [Integer] num_loop number of loops. -1 for infinite loops.
    # @return [Integer] period of displaying loop ID to STDOUT.
    #
    def proposed_display_period(num_loop)
      if num_loop < 0
        display_period = 10000
      elsif num_loop < 100
        display_period = 1
      else
        display_period = 10**((Math.log10(num_loop)-1.5).to_i)
      end
      return display_period
    end
    private :proposed_display_period

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
      message = "#{function_name} returned #{ANL.status_to_string(status)}."

      case status
      when ANL::ANLStatus_critical_error_to_finalize_from_exception, ANL::ANLStatus_critical_error_to_terminate_from_exception
        message += "\n"
        message += "The returned status indicates the presence of a previously caught exception.\n"
        message += "See also that error message displayed above for details."
      end

      case status
      when AS_OK
        return true
      when ANL::ANLStatus_critical_error_to_finalize, ANL::ANLStatus_critical_error_to_finalize_from_exception
        if @stage == :initialization_done || @stage == :analysis_done
          @stage = :error_to_finalize
        else
          @stage = :error_to_terminate
        end
        raise message
      when ANL::ANLStatus_critical_error_to_terminate, ANL::ANLStatus_critical_error_to_terminate_from_exception
        @stage = :error_to_terminate
        raise message
      else
        puts message
      end
      return false
    end
    private :check_status

    # Execute the ANL definition stage.
    #
    def define()
      check_parameter_configuration()
      return @anl_manager if self.definition_already_done?

      if @num_parallels > 1
        @anl_manager = ANL::ANLManagerMT.new(@num_parallels)
      else
        @anl_manager = ANL::ANLManager.new
      end

      vec = ANL::ModuleVector.new(@module_list)
      @anl_manager.set_modules(vec)

      @stage = :definition
      status = @anl_manager.define_analysis()
      unless check_status(status, "define_analysis()")
        @stage = :error_to_terminate
        return
      end
      @stage = :definition_done

      return @anl_manager
    rescue StandardError
      @stage = :error_to_terminate
      raise
    end

    def load_all_parameters()
      check_parameter_configuration()
      if not self.definition_already_done?
        raise "Definition stage is not completed."
      end
      raise 'Parameter application is already in progress.' if @applying_parameters

      begin
        @applying_parameters = true
        until @parameter_setter_list.empty?
          @parameter_setter_list.first.call
          @parameter_setter_list.shift
        end
      rescue StandardError
        @stage = :error_to_terminate
        raise
      ensure
        @applying_parameters = false
      end
    end

    # Run the ANL analysis.
    #
    # @param [Integer] num_loop number of loops. :all or -1 for infinite loops.
    #
    def run(num_loop)
      unless @stage.nil? || @stage == :definition_done
        raise "Analysis cannot start at this stage: #{@stage}."
      end
      if num_loop == :max || num_loop == :all
        num_loop = -1
      end

      @display_period ||= proposed_display_period(num_loop)

      if num_loop == -1
        ptr_size = [nil].pack('p').size # size of nullptr (= size of size_t)
        nbits = ptr_size * 8
        num_loop = (1 << nbits) - 1 # maximum value of size_t
      end

      anl_manager = define()
      return unless anl_manager

      load_all_parameters()

      if parameters_json_filename() && parameters_json_master()
        File.open(parameters_json_filename(), 'w') do |fout|
          fout.print(JSON.pretty_generate(parameters_to_object()))
        end
      end

      @stage = :pre_initialization
      status = anl_manager.pre_initialize_analysis()
      check_status(status, "pre_initialize_analysis()") or return
      @stage = :pre_initialization_done

      if @modification_block
        @modification_block.(self)
      end
      load_all_parameters()

      if parameters_json_filename() && !parameters_json_master()
        anl_manager.parameters_to_json(parameters_json_filename())
      end

      @stage = :initialization
      status = anl_manager.initialize_analysis()
      check_status(status, "initialize_analysis()") or return
      @stage = :initialization_done

      puts ""
      puts "<Begin Analysis> | Time: " + Time.now.to_s
      $stdout.flush
      anl_manager.set_display_period(@display_period)
      status = anl_manager.run_analysis(num_loop, @console)
      puts ""
      puts "<End Analysis>   | Time: " + Time.now.to_s
      $stdout.flush

      if check_status(status, "run_analysis()")
        @stage = :analysis_done
      end

      status = anl_manager.finalize_analysis()
      check_status(status, "finalize_analysis()") or return
      @stage = :finalization_done
    rescue StandardError => ex
      @stage = :error_to_terminate unless @stage == :error_to_finalize
      print_exception(ex)
      if @stage == :error_to_finalize
        status = anl_manager.finalize_analysis()
        puts "finalize_analysis() returned #{ANL.status_to_string(status)}."
      end
      raise
    end

    # Start the ANL interactive session for running the ANL analysis.
    #
    def run_interactive()
      unless @stage.nil? || @stage == :definition_done
        raise "Analysis cannot start at this stage: #{@stage}."
      end
      anl_manager = define()
      return unless anl_manager

      load_all_parameters()

      status = anl_manager.do_interactive_comunication()
      check_status(status, "do_interactive_comunication()") or return
      load_all_parameters()

      if parameters_json_filename() && parameters_json_master()
        File.open(parameters_json_filename(), 'w') do |fout|
          fout.print(JSON.pretty_generate(parameters_to_object()))
        end
      end

      @stage = :pre_initialization
      status = anl_manager.pre_initialize_analysis()
      check_status(status, "pre_initialize_analysis()") or return
      @stage = :pre_initialization_done
      load_all_parameters()

      if parameters_json_filename() && !parameters_json_master()
        anl_manager.parameters_to_json(parameters_json_filename())
      end

      @stage = :initialization
      status = anl_manager.initialize_analysis()
      check_status(status, "initialize_analysis()") or return
      @stage = :initialization_done

      status = anl_manager.do_interactive_analysis()
      if check_status(status, "do_interactive_analysis()")
        @stage = :analysis_done
      end

      status = anl_manager.finalize_analysis()
      check_status(status, "finalize_analysis()") or return
      @stage = :finalization_done
    rescue StandardError => ex
      @stage = :error_to_terminate unless @stage == :error_to_finalize
      print_exception(ex)
      if @stage == :error_to_finalize
        status = anl_manager.finalize_analysis()
        puts "finalize_analysis() returned #{ANL.status_to_string(status)}."
      end
      raise
    end

    def print_exception(ex)
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
      puts "---- displayed by <Ruby> AnalysisChain#print_exception() ----"
      puts ""
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
          if type=="record_map" || type=="record_vector"
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

          if container_type == :record_map
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
          elsif container_type == :record_vector
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
      out.puts "require 'anlnext'"
      out.puts "require #{package.to_s.inspect}"
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
          param.type_name != 'record_map' && param.type_name != 'record_vector'
        }.map do |param|
          type = param.type_name
          default_string =
            case type
            when 'vector<string>', 'list<string>'
              [param.default_string].inspect
            when 'vector<int>', 'vector<double>'
              '[]'
            when *MAP_PARAMETER_TYPES.keys
              param.get_value_auto.inspect
            when 'string'
              param.value_string.inspect
            when 'unsigned_integer'
              param.get_value_unsigned_integer.to_s
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
          "#{param.name.inspect} => #{default_string}"
        end.join(",\n                    ") + ")"
        out.puts with_parameters_string

        mod.parameter_list.select{|param|
          param.type_name == 'record_map'
        }.each do |param|
          default_string = param.default_string
          out.puts "    insert_to_map #{param.name.inspect}, #{default_string.inspect}, {"
          param.num_value_elements.times do |i|
            map_value = param.value_element_info(i)
            type = map_value.type_name
            default_string =
              case type
              when 'string'
                map_value.value_string.inspect
              when 'bool'
                map_value.get_value_auto.inspect
              when 'unsigned_integer'
                map_value.get_value_unsigned_integer.to_s
              when 'double'
                map_value.value_string.to_f.to_s
              else
                map_value.value_string
              end
            out.puts "      #{map_value.name.inspect} => #{default_string},"
          end
          out.puts '    }'
        end

        mod.parameter_list.select{|param|
          param.type_name == 'record_vector'
        }.each do |param|
          default_string = param.default_string
          out.puts "    push_to_vector #{param.name.inspect}, {"
          param.num_value_elements.times do |i|
            map_value = param.value_element_info(i)
            type = map_value.type_name
            default_string =
              case type
              when 'string'
                map_value.value_string.inspect
              when 'bool'
                map_value.get_value_auto.inspect
              when 'unsigned_integer'
                map_value.get_value_unsigned_integer.to_s
              when 'double'
                map_value.value_string.to_f.to_s
              else
                map_value.value_string
              end
            out.puts "      #{map_value.name.inspect} => #{default_string},"
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

end # module ANL
