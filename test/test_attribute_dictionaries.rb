require 'minitest/autorun'
require 'sketchup'

class TestAttributeDictionaries < Minitest::Test

  def setup
    Sketchup.open_file("#{ENV['TEST_RESOURCES']}/Untitled.skp")
    @attribute_dictionaries = Sketchup.active_model.attribute_dictionaries
  end

  def test_object
    assert_kind_of(Sketchup::Entity, @attribute_dictionaries)
    assert_kind_of(Enumerable, @attribute_dictionaries)
  end

  def test_accessor
    assert_instance_of(Sketchup::AttributeDictionary, @attribute_dictionaries['GeoReference'])
    entity = Sketchup.active_model.definitions.find { |d| d.name == 'Heather' }
    entity.attribute_dictionary('Age', 42)
    assert_instance_of(Sketchup::AttributeDictionary, entity.attribute_dictionaries['Age'])
  end

  def test_count
    assert_equal(3, @attribute_dictionaries.count)
  end

  def test_delete_by_dictionary
    assert_equal(3, @attribute_dictionaries.length)
    @attribute_dictionaries.delete(@attribute_dictionaries['GeoReference'])
    assert_equal(2, @attribute_dictionaries.length)
  end
  
  def test_delete_by_name
    assert_equal(3, @attribute_dictionaries.length)
    @attribute_dictionaries.delete('GeoReference')
    assert_equal(2, @attribute_dictionaries.length)
  end

  def test_each
    counter = 0
    @attribute_dictionaries.each { |attribute_dictionary|
      assert_instance_of(Sketchup::AttributeDictionary, attribute_dictionary)
      counter += 1
    }
    assert_equal(3, counter)
  end

  def test_length
    assert_equal(3, @attribute_dictionaries.length)
  end

  def test_count
    assert_equal(3, @attribute_dictionaries.count)
  end

  def test_size
    assert_equal(3, @attribute_dictionaries.size)
  end

end
