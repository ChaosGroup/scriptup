require_relative 'helper'

class TestEntity < Minitest::Test
  UNSUPPORTED = %i[add_observer remove_observer].freeze # no observers in the C API

  def setup
    @model = open_model
    @entity = @model.definitions['Heather']
  end

  def test_unsupported
    assert_unsupported(@entity, UNSUPPORTED)
  end

  def test_attribute_dictionaries
    assert_instance_of(Sketchup::AttributeDictionaries, @entity.attribute_dictionaries)
  end

  def test_attribute_dictionary
    assert_instance_of(Sketchup::AttributeDictionary, @entity.attribute_dictionary('SU_DefinitionSet'))
    assert_nil(@entity.attribute_dictionary('Foo'))
    created = @entity.attribute_dictionary('Bar', true)
    assert_instance_of(Sketchup::AttributeDictionary, created)
    assert_equal('Bar', created.name)
    assert_equal(created, @entity.attribute_dictionary('Bar'))
    assert_raises(TypeError) { @entity.attribute_dictionary(1) }
  end

  def test_get_attribute
    assert_nil(@entity.get_attribute('Nope', 'key'))
    assert_equal(42, @entity.get_attribute('Nope', 'key', 42))
    @entity.set_attribute('Test', 'number', 7)
    assert_equal(7, @entity.get_attribute('Test', 'number'))
    assert_equal(42, @entity.get_attribute('Test', 'missing', 42))
    assert_raises(ArgumentError) { @entity.get_attribute('Test') }
  end

  def test_set_attribute
    assert_equal(42, @entity.set_attribute('Foo', 'Bar', 42))
    assert_equal('text', @entity.set_attribute('Foo', 'String', 'text'))
    assert_equal(true, @entity.set_attribute('Foo', 'Bool', true))
    assert_equal(1.5, @entity.set_attribute('Foo', 'Float', 1.5))
    assert_equal(42, @entity.get_attribute('Foo', 'Bar'))
    assert_equal('text', @entity.get_attribute('Foo', 'String'))
    assert_equal(true, @entity.get_attribute('Foo', 'Bool'))
    assert_equal(1.5, @entity.get_attribute('Foo', 'Float'))
  end

  def test_delete_attribute
    assert_raises(ArgumentError) { @entity.delete_attribute }
    assert_raises(ArgumentError) { @entity.delete_attribute('SU_InstanceSet') }
    assert_raises(ArgumentError) { @entity.delete_attribute('SU_DefinitionSet') }
    refute(@entity.delete_attribute('Test'))
    assert(@entity.delete_attribute('temp'))
    refute(@entity.delete_attribute('temp'))
    assert_raises(NotImplementedError) { @entity.delete_attribute('Foo', 'key') }
  end

  def test_deleted_and_valid
    assert_equal(false, @entity.deleted?)
    assert(@entity.valid?)
  end

  def test_entityID
    assert_instance_of(Integer, @entity.entityID)
    assert(@entity.entityID.positive?)
  end

  def test_persistent_id
    assert_equal(35_379, @entity.persistent_id)
  end

  def test_model
    assert_instance_of(Sketchup::Model, @entity.model)
  end

  def test_parent
    assert_instance_of(Sketchup::DefinitionList, @entity.parent)
  end

  def test_typename
    assert_equal('ComponentDefinition', @entity.typename)
    assert_equal('Material', @model.materials[0].typename)
  end

  def test_equality
    same = @model.definitions['Heather']
    refute_same(@entity, same)
    assert(@entity == same)
    assert(@entity.eql?(same))
    assert_equal(@entity.hash, same.hash)
    refute(@entity == @model.materials[0])
    refute(@entity == 'x')
    assert_equal(1, [@entity, same].uniq.size)
  end

  def test_strings
    assert_instance_of(String, @entity.inspect)
    assert_instance_of(String, @entity.to_s)
  end
end
