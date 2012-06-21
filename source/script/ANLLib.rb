# = ANLLib
# Author:: Hirokazu Odaka
# 
# This is a class library written in Ruby for the ANL Next framework
#

require 'anl'
require 'rexml/document'


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


def vec(x, y, z=nil)
  return Vector.new(x, y, z)
end


class ANLApp
  def initialize(module_list = [], n_loop = -1, display_frequency = 1000)
    @module_list = module_list
    @module_hash = {}
    @current_module = nil
    @n_loop = n_loop
    @display_frequency = display_frequency
    @set_param_list = []
    @set_module_list = []
    @is_startup = false
  end
  
  attr_writer :module_list, :n_loop, :display_frequency
  attr_reader :n_loop, :display_frequency, :is_startup

  def push(anl_module)
    @module_list << anl_module
    sym = anl_module.module_name.to_sym
    unless @module_hash.has_key? sym
      @module_hash[sym] = anl_module
    end
    @current_module = anl_module
  end

  def chain(anl_module_class, module_name=nil, module_version=nil)
    mod = nil
    if module_version
      instance_eval %{
        mod = #{anl_module_class}.new(module_name, module_version)
      }
    elsif module_name
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

  def expose_module(anl_module_symbol)
    @current_module = get_module(anl_module_symbol)
  end

  def get_module(anl_module_symbol)
    mod = @module_hash[anl_module_symbol]
    if mod==nil
      raise 'Module symbol '+anl_module_symbol.to_s+' is not registered.'
    end
    return mod
  end

  def text(description)
    @current_module.set_module_description(description)
  end

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
  end

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
  end

  def set_parameters(anl_module_symbol, parameters={}, &set_param)
    mod = expose_module(anl_module_symbol)
    @set_module_list << mod
    parameters.each{|name, value|
      setp(name, value)
    }
    set_param.call(mod) if block_given?
  end
  
  def set_loop(n_loop=nil, display_frequency=nil)
    if n_loop
      @n_loop = n_loop
      if display_frequency
        @display_frequency = display_frequency
      else
        @display_frequency = n_loop/100
      end
    else
      @n_loop = -1
      if display_frequency
        @display_frequency = display_frequency
      else
        @display_frequency = 1000
      end
    end
  end

  def startup()
    @anl = Anl::ANLNext.new
    vec = Anl::ModuleVector.new(@module_list)
    @anl.SetModules(vec)
    status = @anl.Startup()
    status == Anl::AS_OK or raise "Startup() returned "+status.to_s
    @is_startup = true
    return @anl
  end

  def run(n_loop=nil, display_frequency=nil, &set_param)
    set_loop(n_loop, display_frequency)
    
    anl = startup()
    
    while s = @set_param_list.shift
      s.call
    end
    set_param.call if block_given?

    status = anl.Prepare()
    status == Anl::AS_OK or raise "Prepare() returned "+status.to_s

    if @display_frequency==0
      if @n_loop>0
        @display_frequency = n/100
      else
        @display_frequency = 10000
      end
    end
  
    status = anl.Initialize()
    status == Anl::AS_OK or raise "Initialize() returned "+status.to_s

    puts "\nAnalysis Begin  | Time: " + Time.now.to_s
    status = anl.Analyze(@n_loop, @display_frequency)
    status == Anl::AS_OK or raise "Analyze() returned "+status.to_s
    puts "Analysis End    | Time: " + Time.now.to_s
    
    status = anl.Exit()
    status == Anl::AS_OK or raise "Exit() returned "+status.to_s
  rescue RuntimeError => ex
    puts ""
    puts "  ### ANL NEXT Exception ###  "
    puts ""
    puts ex
  end
  
  def runInteractive(&set_param)
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

  def print_all_param()
    anl = startup()
    @module_list.each{|m|
      puts "--- "+m.module_name+" ---"
      m.print_parameters
      puts ''
    }
  end

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
