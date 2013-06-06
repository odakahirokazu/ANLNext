# ANLLib
# Class library written in Ruby for the ANL Next framework.
#
# @author: Hirokazu Odaka
#

require 'anl'
require 'rexml/document'


# Get a list of parameters registered in the given module.
#
# @param [ANLModule] anl_module 
# @return [Array] list of parameters registered in the given module.
#
def parameter_list(anl_module)
  l = []
  i = anl_module.ModParamBegin
  e = anl_module.ModParamEnd
  while i != e
    l << i.value
    i.next
  end
  return l
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


# A utility method to make a Vector object.
#
# @param [Float] x first component of the vector.
# @param [Float] y second component of the vector.
# @param [Float] z third component of the vector.
# @return [Vector] a Vector object (x, y, z)
#
def vec(x, y, z=nil)
  return Vector.new(x, y, z)
end


# Class of an ANL Application.
# @author Hirokazu Odaka
#
class ANLApp
  # Initialization method.
  #
  # @param [Array] module_list a list of ANL modules.
  #
  def initialize(module_list = [])
    @module_list = module_list
    @module_hash = {}
    @current_module = nil
    @n_loop = -1
    @display_frequency = 1000
    @set_param_list = []
    @set_module_list = []
    @is_startup = false
    @thread_mode = true
  end
  
  attr_accessor :n_loop, :display_frequency, :thread_mode
  attr_reader :is_startup
  attr_writer :module_list
  
  
  # Push an ANL module to the module chain.
  #
  # @param [ANLModule] anl_module ANL module to be pushed.
  # @return [ANLModule] ANL module pushed.
  #
  def push(anl_module)
    @module_list << anl_module
    sym = anl_module.module_name.to_sym
    unless @module_hash.has_key? sym
      @module_hash[sym] = anl_module
    end
    @current_module = anl_module
  end

  # Insert an ANL module to the module chain at a specified position.
  #
  # @param [Fixnum] index position.
  # @param [ANLModule] anl_module ANL module to be inserted.
  # @return [ANLModule] ANL module inserted.
  #
  def insert(index, anl_module)
    @module_list.insert(index, anl_module)
    sym = anl_module.module_name.to_sym
    unless @module_hash.has_key? sym
      @module_hash[sym] = anl_module
    end
    @current_module = anl_module
  end

  # Push an ANL module that is specified by a symbol to the module chain.
  #
  # @param [Symbol] anl_module_class ANL module to be pushed.
  # @param [String] module_name ANL module name.
  # @return [ANLModule] ANL module pushed.
  #
  def chain(anl_module_class, module_name=nil)
    mod = nil
    if module_name
      instance_eval %{
        mod = #{anl_module_class}.new(module_name)
      }
    else
      instance_eval %{
        mod = #{anl_module_class}.new
      }
    end

    push(mod)
  end

  # Get an ANL module by module symbol and set it to the current module.
  #
  # @param [Symbol] anl_module_symbol ANL module.
  # @return [ANLModule] ANL module.
  #
  def expose_module(anl_module_symbol)
    @current_module = get_module(anl_module_symbol)
  end

  # Get an ANL module by module symbol.
  # This method does not change the current module.
  #
  # @param [Symbol] anl_module_symbol ANL module.
  # @return [ANLModule] ANL module.
  #
  def get_module(anl_module_symbol)
    mod = @module_hash[anl_module_symbol]
    if mod==nil
      raise 'Module symbol '+anl_module_symbol.to_s+' is not registered.'
    end
    return mod
  end

  # Get position of the first ANL module which has the specified name
  # in the analysis chain.
  #
  # @param [Symbol] anl_module_symbol ANL module.
  # @return [Fixnum] Position.
  #
  def index(anl_module_symbol)
    @module_list.index{|mod| mod.module_name.to_sym==anl_module_symbol }
  end

  # Get position of the last ANL module which has the specified name
  # in the analysis chain.
  #
  # @param [Symbol] anl_module_symbol ANL module.
  # @return [Fixnum] Position.
  #
  def rindex(anl_module_symbol)
    @module_list.rindex{|mod| mod.module_name.to_sym==anl_module_symbol }
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
          lambda { mod.set_vector(name, value.x, value.y, value.z) }
        else
          lambda { mod.set_vector(name, value.x, value.y) }
        end
      elsif value.class == Array
        if value.empty?
          lambda { mod.clear_array(name) }
        else
          f = value.first
          if f.class == String
            lambda { mod.set_svec(name, value) }
          elsif f.class == Float
            lambda { mod.set_fvec(name, value) }
          elsif f.integer?
            lambda { mod.set_ivec(name, value) }
          else
            raise "setp exception: type invalid (Array of *)"
          end
        end
      else
        lambda { mod.set_param(name, value) }
      end
    
    if @is_startup
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
    set_param = lambda{
      mod.param_map_insert(map_name, key) {|v|
        map_values.each{|value_name, value|
          v.set_map_value(value_name, value)
        }
      }
    }
    
    if @is_startup
      set_param.call
    else
      @set_param_list << set_param
    end

    return
  end

  # Utility method to set parameters of the current module.
  # Before the ANL startup session, this method reserves parameter setting,
  # and acctual setting to the ANL module object is performed after the
  # startup session.
  # If this method is called after the startup session, parameter setting is
  # performed immediately.
  #
  # @param [Symbol] anl_module_symbol Symbol of ANL module.
  # @param [Hash] parameters list of parameters (name, value).
  # @yield [mod] a block can be given for additional process.
  # @yieldparam mod the current module is given.
  #
  def set_parameters(anl_module_symbol, parameters={}, &set_param)
    mod = expose_module(anl_module_symbol)
    @set_module_list << mod unless @set_module_list.include? mod
    parameters.each{|name, value|
      setp(name, value)
    }
    set_param.call(mod) if block_given?

    return
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
  
  # Execute the ANL startup session.
  #
  def startup()
    @anl = Anl::ANLManager.new
    vec = Anl::ModuleVector.new(@module_list)
    @anl.SetModules(vec)
    status = @anl.Startup()
    status == Anl::AS_OK or raise "Startup() returned "+status.to_s
    @is_startup = true
    return @anl
  end

  # Run the ANL analysis.
  #
  # @param [Fixnum] n_loop number of loop. -1 for infinite loop.
  # @param [Fixnum] display_frequency frequency of displaying loop ID
  #     to STDOUT.
  # @yield [mod] a block can be given for additional process.
  # @yieldparam mod the current module is given.
  #
  def run(n_loop=nil, display_frequency=nil, &set_param)
    set_loop(n_loop, display_frequency)
    
    anl = startup()
    
    while s = @set_param_list.shift
      s.call
    end
    set_param.call if block_given?

    status = anl.Prepare()
    status == Anl::AS_OK or raise "Prepare() returned "+status.to_s

    status = anl.Initialize()
    status == Anl::AS_OK or raise "Initialize() returned "+status.to_s

    puts "\nAnalysis Begin  | Time: " + Time.now.to_s
    $stdout.flush
    anl.SetDisplayFrequency(@display_frequency)
    status = anl.Analyze(@n_loop, @thread_mode)
    
    status == Anl::AS_OK or raise "Analyze() returned "+status.to_s
    puts "Analysis End    | Time: " + Time.now.to_s
    $stdout.flush
    
    status = anl.Exit()
    status == Anl::AS_OK or raise "Exit() returned "+status.to_s
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
    anl = startup()

    @set_param_list.each{|s| s.call }
    set_param.call if block_given?
    @set_module_list.each{|mod|
      status = mod.mod_prepare()
      status == Anl::AS_OK or raise mod.name+" :: mod_prepare() returned "+status.to_s
    }
    
    status = anl.InteractiveCom()
    status == Anl::AS_OK or raise "InteractiveCom() returned "+status.to_s

    status = anl.InteractiveAna()
    status == Anl::AS_OK or raise "InteractiveAna() returned "+status.to_s
    
    status = anl.Exit()
    status == Anl::AS_OK or raise "Exit() returned "+status.to_s
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
      puts "--- "+m.module_name+" ---"
      m.print_parameters
      puts ''
    }
  end

  # Make a XML document describing module parameters.
  #
  # @param [STRING] file_name output XML file name. If nil, output to STDOUT.
  # @param [STRING] category
  #
  def make_doc(file_name = nil, category="")
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
      parameter_list(mod).each {|param|
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
    if file_name
      File::open(file_name, 'w') {|fo|
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
  def make_script(file_name = nil, include_text="",
                  n_loop=10000, display_frequency=1000)
    out = file_name ? File::open(file_name, 'w') : STDOUT
    out.puts '#!/usr/bin/env ruby'
    out.puts "require 'ANLLib'"
    out.puts ''
    out.puts include_text
    out.puts ''
    out.puts 'n_loop = '+n_loop.to_s
    out.puts 'display_frequency = '+display_frequency.to_s
    out.puts ''
    out.puts 'anl = ANLApp.new'
    
    @module_list.each{|mod|
      out.puts 'anl.chain :'+mod.module_name
    }

    out.puts ''
    @module_list.each{|mod|
      mapList = []
      
      out.puts 'anl.set_parameters :'+mod.module_name+', {'
      parameter_list(mod).each {|param|
        type = param.type_name
        if type=='map'
          mapList << param
        else
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
          out.puts '  "'+param.name+'" => '+default_string+","
        end
      }
      out.puts '}'

      mapList.each {|param|
        default_string = param.default_string
        out.puts 'anl.insert_map "'+param.name+'", "'+default_string+'", {'
        param.num_map_value.times{|i|
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
        }
        out.puts '}'
      }
      
      out.puts ''
    }

    out.puts "anl.run(n_loop, display_frequency)"
    out.close if file_name
  end
end
