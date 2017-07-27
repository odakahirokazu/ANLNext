# anlnext/utility.rb
# Class library written in Ruby for the ANL Next framework.
#
# @author: Hirokazu Odaka
#

module ANL

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
      @importModules.each do |s|
        if s.is_a? Hash
          puts '%import(module="'+s[:name]+'") "'+s[:file]+'"'
        else
          puts '%import "'+s+'"'
        end
      end
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
