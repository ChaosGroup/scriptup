require_relative 'helper'

class TestClassifications < Minitest::Test
  UNSUPPORTED = %i[unload_schema].freeze # no C API

  # Schema files aren't in test-resources; use a local SketchUp install when present.
  SCHEMA = Dir.glob(File.join(Dir.home, 'Library/Application Support/SketchUp */SketchUp/Classifications/IFC 2x3.skc')).first

  def setup
    @model = open_model
    @classifications = @model.classifications
  end

  def test_unsupported
    assert_unsupported(@classifications, UNSUPPORTED)
  end

  def test_class
    assert_instance_of(Sketchup::Classifications, @classifications)
    assert_kind_of(Enumerable, @classifications)
  end

  def test_counts_and_keys
    assert_equal(@classifications.keys.size, @classifications.length)
    assert_equal(@classifications.keys.size, @classifications.size)
    assert_nil(@classifications['No such schema'])
  end

  def test_load_schema_failure
    refute(@classifications.load_schema('/does/not/exist.skc'))
    refute(@model.definitions['Heather'].add_classification('No such schema', 'IfcWall'))
  end

  def test_loaded_schema
    skip 'no IFC schema file installed' unless SCHEMA
    assert(@classifications.load_schema(SCHEMA))
    name = @classifications.keys.first
    schema = @classifications[name]
    assert_instance_of(Sketchup::ClassificationSchema, schema)
    assert_equal(name, schema.name)
    assert_equal(schema, @classifications[0])
    assert_kind_of(Comparable, schema)
    assert_equal(0, schema <=> schema)
    assert_nil(schema <=> 1)
    refute_respond_to(schema, :namespace) # no C API
    seen = []
    assert_nil(@classifications.each { |s| seen << s })
    assert_includes(seen, schema)
    assert(@model.definitions['Heather'].add_classification(name, 'IfcWall'))
  end
end
