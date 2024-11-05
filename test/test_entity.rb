require 'minitest/autorun'
require 'sketchup'

class TestEntity < Minitest::Test

  def setup
    Sketchup.open_file("#{ENV['TEST_RESOURCES']}/Untitled.skp")
    @entity = Sketchup.active_model.definitions.find { |d| d.name == 'Heather' }
  end

  def test_add_observer
    assert(@entity.add_observer(nil))
  end

  def test_attribute_dictionaries
    assert_instance_of(Sketchup::AttributeDictionaries, @entity.attribute_dictionaries)
  end

  def test_attribute_dictionary
    assert_instance_of(Sketchup::AttributeDictionary, @entity.attribute_dictionary('SU_DefinitionSet'))
    assert_nil(@entity.attribute_dictionary('Foo'))
    assert_instance_of(Sketchup::AttributeDictionary, @entity.attribute_dictionary('Bar', true))
  end

  def test_delete_attribute
    assert_raises(ArgumentError) {
      @entity.delete_attribute()
    }
    assert_raises(ArgumentError) {
      @entity.delete_attribute('SU_InstanceSet')
    }
    assert_raises(ArgumentError) {
      @entity.delete_attribute('SU_DefinitionSet')
    }
    refute(@entity.delete_attribute('Test'))
    assert(@entity.delete_attribute('temp'))
    refute(@entity.delete_attribute('temp'))
    skip
    assert(@entity.delete_attribute('SU_DefinitionSet', 'price'))
    assert(@entity.delete_attribute('SU_DefinitionSet'))
  end

  def test_deleted?
    assert_equal(false, @entity.deleted?)
    # TODO: test deletion
  end

  # rubocop:disable Naming/MethodName
  def test_entityID
    assert_instance_of(Integer, @entity.entityID)
    assert(@entity.entityID.positive?)
  end
  # rubocop:enable Naming/MethodName

  def test_get_attribute
    skip
    assert_equal('', @entity.get_attribute('SU_DefinitionSet', 'Price'))
    assert_equal(42, @entity.get_attribute('SU_DefinitionSet', 'Foo', 42))
  end

  def test_inspect
    assert_instance_of(String, @entity.inspect)
  end

  def test_model
    assert_instance_of(Sketchup::Model, @entity.model)
  end

  def test_parent
    assert_instance_of(Sketchup::DefinitionList, @entity.parent)
  end

  def test_persistent_id
    assert_instance_of(Integer, @entity.persistent_id)
    assert_equal(35379, @entity.persistent_id)
  end

  def test_remove_observer
    assert(@entity.remove_observer(42) == false)
  end

  def test_set_attribute
    skip
    assert_equal(42, @entity.set_attribute('Foo', 'Bar', 42))
  end

  def test_to_s
    assert_instance_of(String, @entity.to_s)
  end

  def test_typename
    assert_equal('ComponentDefinition', @entity.typename)
  end

  def test_valid?
    assert(@entity.valid?)
  end

end
