#!/usr/bin/ruby

defFileName = ARGV[0]
useROOT = ARGV[1].to_i
sourceFileName = ARGV[2]

unless sourceFileName.class == String
  sourceFileName = defFileName.sub(".def", ".cc")
end

headerFileName = defFileName.sub(".def", ".hh")

module_list = []
File::open(defFileName) {|f|
  while line = f.gets
    next if line[0, 1] == '#'
    module_list << line.chomp
  end
}


File::open(sourceFileName, 'w') { |fo|
  fo.puts '// ' + File.basename(sourceFileName)
  fo.puts '// ANL NEXT framework'
  fo.puts '// '
  fo.puts ''
  fo.puts '#include <vector>'
  fo.puts '#include "ANLNext.hh"'
  fo.puts '#include "ANLVModule.hh"'
  fo.puts '#include "ANLException.hh"'
  fo.puts ''
  fo.puts '#include "' + File.basename(headerFileName) + '"'
  fo.puts ''
  fo.puts '#include "TROOT.h"' if useROOT
  fo.puts ''
  fo.puts 'using namespace anl;'
  fo.puts ''
  fo.puts 'int main()'
  fo.puts '{'
  if useROOT
    fo.puts '  TROOT troot("TROOT", "ANL NEXT TROOT");'
    fo.puts ''
  end
  fo.puts '  ANLNext anl;'
  fo.puts ''
  fo.puts '  std::vector<ANLVModule*> modules;'
  module_list.each {|mod|
    fo.puts '  modules.push_back(new ' + mod + ');';
  }
  fo.puts ''
  fo.puts '  try {'
  fo.puts '    anl.SetModules(modules);'
  fo.puts '    anl.Startup();'
  fo.puts '    if (anl.InteractiveCom()==AS_OK) {'
  fo.puts '      anl.InteractiveAna();'
  fo.puts '      anl.Exit();'
  fo.puts '    }'
  fo.puts '  }'
  fo.puts '  catch (const ANLException& ex) {'
  fo.puts '    std::cout << ex.print() << std::endl;'
  fo.puts '  }'
  fo.puts ''
  fo.puts '  for (size_t i=0; i<modules.size(); ++i) {'
  fo.puts '    delete modules[i];'
  fo.puts '    modules[i] = 0;'
  fo.puts '  }'
  fo.puts '}'
}
