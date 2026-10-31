require 'anlnext'
require 'tmpdir'

ParameterFixture = Struct.new(:name, :type_name, :value_string, :default_string, :elements) do
  def num_value_elements
    elements.size
  end

  def value_element_info(index)
    elements[index]
  end
end

ModuleFixture = Struct.new(:module_name, :module_id, :parameter_list)

# Execute the generated setup without starting an analysis or loading a C++ fixture.
class GeneratedStringApp < ANL::ANLApp
  class << self
    attr_accessor :captured
  end

  def run(_num_loop)
    @captured = {}
    setup
    self.class.captured = @captured
  end

  def add_namespace(_namespace); end

  def chain(name)
    @captured[:module] = name
  end

  def with_parameters(values)
    @captured[:parameters] = values
  end

  def insert_to_map(name, key, values)
    @captured[:map] = [name, key, values]
  end

  def push_to_vector(name, values)
    @captured[:vector] = [name, values]
  end
end

payload = %q(a"b\c#{1+1}) + "\n\t\0雪"
string_name = "message-#{payload}"
vector_name = "strings-#{payload}"
list_name = "list-#{payload}"
map_name = "table-#{payload}"
rows_name = "rows-#{payload}"
element_name = "element-#{payload}"
key = "key-#{payload}"
element = ParameterFixture.new(element_name, 'string', payload)
parameters = [
  ParameterFixture.new(string_name, 'string', payload),
  ParameterFixture.new(vector_name, 'vector<string>', nil, payload),
  ParameterFixture.new(list_name, 'list<string>', nil, payload),
  ParameterFixture.new(map_name, 'record_map', nil, key, [element]),
  ParameterFixture.new(rows_name, 'record_vector', nil, '', [element])
]
chain = ANL::AnalysisChain.new
chain.push(ModuleFixture.new('StringFixture', 'fixture', parameters))

Dir.mktmpdir do |directory|
  package = File.join(directory, %q(package'"#{1+1}))
  File.write(package + '.rb', '')
  filename = File.join(directory, 'generated.rb')
  chain.make_script(output: filename, package: package,
                    namespace: 'ANL', app_name: 'GeneratedStringApp')
  RubyVM::InstructionSequence.compile(File.read(filename))
  load filename
end

captured = GeneratedStringApp.captured
raise 'module name changed' unless captured[:module] == :StringFixture
raise 'string parameter changed' unless captured[:parameters][string_name] == payload
raise 'vector string default changed' unless captured[:parameters][vector_name] == [payload]
raise 'list string default changed' unless captured[:parameters][list_name] == [payload]
raise 'record map strings changed' unless captured[:map] == [map_name, key, { element_name => payload }]
raise 'record vector strings changed' unless captured[:vector] == [rows_name, { element_name => payload }]
