# ANLLib
# Class library written in Ruby for the ANL Next framework.
#
# @author: Hirokazu Odaka
#

require 'ANL'
require 'rexml/document'

module ANL
  class BasicModule
    # Get a list of parameters registered in the module.
    #
    # @return [Array] list of parameters registered in the given module.
    #
    def parameter_list()
      l = []
      i = self.ModParamBegin
      e = self.ModParamEnd
      while i != e
        l << i.value
        i.next
      end
      return l
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
      ANL::AS_OK => "AS_OK",
      ANL::AS_SKIP => "AS_SKIP",
      ANL::AS_SKIP_ERR => "AS_SKIP_ERR",
      ANL::AS_QUIT => "AS_QUIT",
      ANL::AS_QUIT_ERR => "AS_QUIT_ERR",
    }[i] or "unknown status"
  end
  module_function :show_status


  # Class of ANL module + its parameters
  #
  class ModuleInitializer
    def initialize(anl_module_class, module_id=nil, parameters=nil)
      @module_class = anl_module_class
      @module_id = module_id
      @parameters = parameters
    end
    attr_accessor :module_id
    attr_reader :module_class, :parameters, :set_param_func

    def with_parameters(parameters=nil, &set_param)
      @parameters = parameters if parameters
      @set_param_func = set_param
    end
  end


  # Class of an ANL Application.
  #
  class ANLApp
    # Initialization method.
    #
    # @param [Array] module_list a list of ANL modules.
    #
    def initialize()
      @module_list = []
      @module_hash = {}
      @current_module = nil
      @n_loop = -1
      @display_frequency = 1000
      @set_param_list = []
      @set_module_list = []
      @startup_done = false
      @thread_mode = true
      @namespace_list = [Object]
    end

    attr_accessor :n_loop, :display_frequency
    attr_accessor :thread_mode
    attr_reader :startup_done

    # Setup analysis chain and set parameters.
    # By default, this method does nothing.
    # A user can redefine this method in a derived class.
    # This method is called at the beginning of run() method.
    #
    def setup()
      nil
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
      set_param =
        if value.class == Vector
          if value.z
            lambda{ mod.set_vector(name, value.x, value.y, value.z) }
          else
            lambda{ mod.set_vector(name, value.x, value.y) }
          end
        elsif value.class == Array
          if value.empty?
            lambda{ mod.clear_array(name) }
          else
            f = value.first
            if f.class == String
              lambda{ mod.set_svec(name, value) }
            elsif f.class == Float
              lambda{ mod.set_fvec(name, value) }
            elsif f.integer?
              lambda{ mod.set_ivec(name, value) }
            else
              raise "ANLApp#setp(): type invalid (Array of *)."
            end
          end
        else
          lambda{ mod.set_param(name, value) }
        end

      if @startup_done
        set_param.call
      else
        @set_param_list << set_param
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
    def insert_map(map_name, key, map_values)
      mod = @current_module
      set_param = lambda do
        mod.param_map_insert(map_name, key) do |v|
          map_values.each do |value_name, value|
            v.set_map_value(value_name, value)
          end
        end
      end

      if @startup_done
        set_param.call
      else
        @set_param_list << set_param
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
      @set_module_list << mod unless @set_module_list.include? mod
      if parameters
        parameters.each{|name, value| setp(name.to_s, value) }
      end
      if block_given?
        set_param.(mod)
      end
      return
    end

    # Utility method to push ANL module and then set parameters.
    #
    def chain_with_parameters(initializer)
      chain(initializer.module_class, initializer.module_id)
      with_parameters(initializer.parameters, &initializer.set_param_func)
    end

    # Set loop number and display frequency.
    #
    # @param [Fixnum] n_loop number of loop. -1 for infinite loop.
    # @param [Fixnum] display_frequency frequency of displaying loop ID
    #     to STDOUT.
    #
    def set_loop(n_loop=nil, display_frequency=nil)
      @n_loop = n_loop || -1
      @display_frequency = display_frequency
      if @n_loop < 0
        @display_frequency ||= 10000
      elsif @n_loop < 100
        @display_frequency ||= 1
      else
        @display_frequency ||= 10**((Math.log10(@n_loop)-1.5).to_i)
      end
    end

    # Check ANL status. If it is not OK, raise an exception.
    #
    # param[Fixnum] status ANL status.
    # param[String] function name of ANL routine.
    #
    def check_status(status, function_name)
      unless status == ANL::AS_OK
        raise "#{function_name} returned #{show_status(status)}."
      end
      true
    end

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

    # Run the ANL analysis.
    #
    # @param [Fixnum] n_loop number of loop. -1 for infinite loop.
    # @param [Fixnum] display_frequency frequency of displaying loop ID
    #     to STDOUT.
    # @yield [self] a block can be given for additional process.
    # @yieldparam self
    #
    def run(n_loop=nil, display_frequency=nil)
      setup()

      if n_loop == :all
        n_loop = -1
      end
      set_loop(n_loop, display_frequency)

      yield self if block_given?

      anl = startup()

      while s = @set_param_list.shift
        s.call
      end

      status = anl.Prepare()
      check_status(status, "Prepare()")

      status = anl.Initialize()
      check_status(status, "Initialize()")

      puts ""
      puts "Analysis Begin  | Time: " + Time.now.to_s
      $stdout.flush
      anl.SetDisplayFrequency(@display_frequency)
      status = anl.Analyze(@n_loop, @thread_mode)
      check_status(status, "Analyze()")
      puts ""
      puts "Analysis End    | Time: " + Time.now.to_s
      $stdout.flush

      status = anl.Exit()
      check_status(status, "Exit()")
    rescue RuntimeError => ex
      puts ""
      puts "  ### ANL NEXT Exception ###  "
      puts ""
      puts ex
      raise
    end

    # Start the ANL interactive session for running the ANL analysis.
    #
    # @yield [mod] a block can be given for additional process.
    # @yieldparam mod the current module is given.
    #
    def run_interactive(&set_param)
      setup()
      anl = startup()

      @set_param_list.each{|s| s.call }
      set_param.call if block_given?
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
    def print_all_param()
      anl = startup()
      @module_list.each{|m|
        puts "--- "+m.module_id+" ---"
        m.print_parameters
        puts ''
      }
    end

    # Make a XML document describing module parameters.
    #
    # @param [STRING] filename output XML filename. If nil, output to STDOUT.
    # @param [STRING] category
    #
    def make_doc(filename = nil, category="")
      doc = REXML::Document.new
      doc << REXML::XMLDecl.new('1.0', 'UTF-8')
      node = doc.add_element("anlmodules") #, {"xmlns" => "http://www.w3.org/2010/ANLNext"})
      node.add_element("category").add_text category
      @module_list.each{|mod|
        o = node.add_element("module")
        o.add_element("name").add_text mod.module_name
        o.add_element("version").add_text mod.module_version
        o.add_element("text").add_text mod.module_description
        ps = o.add_element("parameters")
        mod.parameter_list.each {|param|
          type = param.type_name
          map_type = (type=="map") ? "map" : ""

          p = ps.add_element("param", {"map_type" => map_type})
          p.add_element("name").add_text param.name
          p.add_element("type").add_text type
          p.add_element("unit").add_text param.unit_name
          if type=="vector of string" || type=="list of string"
            p.add_element("default_value").add_text param.default_string
          else
            p.add_element("default_value").add_text param.value_string
          end
          p.add_element("description").add_text param.description

          if map_type=="map"
            p = ps.add_element("param", {"map_type" => "key"})
            p.add_element("name").add_text param.map_key_name
            p.add_element("type").add_text "string"
            p.add_element("unit").add_text ""
            p.add_element("default_value").add_text param.default_string
            p.add_element("description").add_text ""

            param.num_map_value.times{|i|
              mapValue = param.get_map_value(i)
              p = ps.add_element("param", {"map_type" => "value"})
              p.add_element("name").add_text mapValue.name
              p.add_element("type").add_text mapValue.type_name
              p.add_element("unit").add_text mapValue.unit_name
              p.add_element("default_value").add_text mapValue.value_string
              p.add_element("description").add_text mapValue.description
            }
          end
        }
      }

      formatter = REXML::Formatters::Pretty.new
      formatter.compact = true
      if filename
        File::open(filename, 'w') {|fo|
          formatter.write(doc, fo)
          fo.puts ''
        }
      else
        formatter.write(doc, STDOUT)
        puts ''
      end
    end

    # Make a Ruby run script template for this ANL chain.
    #
    # @param [STRING] filename output script filename. If nil, output to STDOUT.
    # @param [STRING] package name of the Ruby extension library.
    # @param [STRING] namespace namespace of the Ruby extension module.
    # @param [STRING] app_name name of your original app.
    #
    def make_script(filename = nil, package="myPackage", namespace="MyPackage",
                    app_name="MyApp")
      out = filename ? File::open(filename, 'w') : STDOUT
      out.puts '#!/usr/bin/env ruby'
      out.puts "require 'ANLLib'"
      out.puts "require '#{package}'"
      out.puts ''
      out.puts 'n_loop = 100000'
      out.puts 'display_frequency = 1000'
      out.puts ''
      out.puts "class #{app_name} < ANL::ANLApp"
      out.puts "  def setup()"
      out.puts "    add_namespace #{namespace}"
      out.puts ""
      @module_list.each do |mod|
        out.puts '    chain :'+mod.module_name
        with_parameters_string = '    with_parameters('+ mod.parameter_list.select{|param|
          param.type_name != 'map'
        }.map do |param|
          type = param.type_name
          default_string =
            case type
            when 'vector of string', 'list of string'
              '["'+param.default_string+'"]'
            when 'string'
              '"'+param.value_string+'"'
            when 'float'
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
          out.puts 'anl.insert_map "'+param.name+'", "'+default_string+'", {'
          param.num_map_value.times do |i|
            mapValue = param.get_map_value(i)
            type = mapValue.type_name
            default_string =
            case type
            when 'string'
              '"'+mapValue.value_string+'"'
            when 'float'
              mapValue.value_string.to_f.to_s
            else
              mapValue.value_string
            end
            out.puts '  "'+mapValue.name+'" => '+default_string+","
          end
          out.puts '}'
        end
        out.puts ''
      end

      out.puts "  end"
      out.puts "end"
      out.puts ""
      out.puts "anl = #{app_name}.new"
      out.puts "anl.run(n_loop, display_frequency)"
      out.close if filename
    end
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
