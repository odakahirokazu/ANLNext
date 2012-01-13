#!/usr/bin/ruby

defFileName = ARGV[0]
headerFileName = ARGV[1]

unless headerFileName.class == String
  headerFileName = defFileName.sub(".def", ".hh")
end

File::open(headerFileName, 'w') { |headerFile|
  File::open(defFileName) {|f|
    while line = f.gets
      next if line[0, 1] == '#'
      line.chomp!
      pos = line.rindex('::')
      line = line[(pos+2)..-1] if pos
      headerFile.print "#include \"" + line +  ".hh\"\n"
    end
  }
}
