require_relative 'helper'

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
    assert_instance_of(Sketchup::AttributeDictionary, @attribute_dictionaries[@attribute_dictionaries['GeoReference']])
  end

  def test_count
    assert_equal(3, @attribute_dictionaries.count)
  end

  def test_delete
    dictionary = @attribute_dictionaries['GeoReference']
    assert_equal(dictionary, @attribute_dictionaries.delete('GeoReference'))
    assert_nil(@attribute_dictionaries['GeoReference'])
    assert_nil(@attribute_dictionaries.delete('GeoReference'))
    entity = Sketchup.active_model.definitions['Heather']
    temp = entity.attribute_dictionaries['temp']
    assert_equal(temp, entity.attribute_dictionaries.delete(temp))
    assert_nil(entity.attribute_dictionary('temp'))
  end

  def test_length
    assert_equal(3, @attribute_dictionaries.length)
  end

  def test_size
    assert_equal(3, @attribute_dictionaries.size)
  end

end
