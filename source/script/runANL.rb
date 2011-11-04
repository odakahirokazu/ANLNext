require 'anl'

puts "This ANL library is old. Please use 'ANLLib.rb'."

def runANL(analysis_array, n, display_frequency=0, &setp)
  anl = Anl::ANLNext.new
  vec = Anl::ModuleVector.new(analysis_array)
  anl.SetModules(vec)
  status = anl.Startup()
  status == Anl::AS_OK or raise "Startup() returned "+status.to_s
  
  setp.call

  status = anl.Prepare()
  status == Anl::AS_OK or raise "Prepare() returned "+status.to_s

  if display_frequency==0
    if n>0
      display_frequency = n/100
    else
      display_frequency = 10000
    end
  end
  
  status = anl.Initialize()
  status == Anl::AS_OK or raise "Initialize() returned "+status.to_s

  puts "\nAnalysis Begin  | Time: " + Time.now.to_s
  status = anl.Analyze(n, display_frequency)
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


def runInteractiveANL(analysis_array)
  anl = Anl::ANLNext.new
  vec = Anl::ModuleVector.new(analysis_array)
  anl.SetModules(vec)

  status = anl.Startup()
  status == Anl::AS_OK or raise "Startup() returned "+status.to_s

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


def startup(analysis_array)
  anl = Anl::ANLNext.new
  vec = Anl::ModuleVector.new(analysis_array)
  anl.SetModules(vec)
  status = anl.Startup()
  status == Anl::AS_OK or raise "Startup() returned "+status.to_s
  anl
end


def print_all_param(analysis_array)
  anl = startup(analysis_array)
  analysis_array.each{|m|
    puts "--- "+m.module_name+" ---"
    m.print_parameters
    puts ''
  }
end
