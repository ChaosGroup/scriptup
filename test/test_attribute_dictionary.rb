require_relative 'helper'

class TestAttributeDictionary < Minitest::Test

  def setup
    Sketchup.open_file("#{ENV['TEST_RESOURCES']}/Untitled.skp")
    @attribute_dictionary = Sketchup.active_model.attribute_dictionaries['GeoReference']
  end

  def test_object
    assert_kind_of(Sketchup::Entity, @attribute_dictionary)
    assert_kind_of(Enumerable, @attribute_dictionary)
  end

  def test_getter
    assert_equal(40.018309, @attribute_dictionary['Latitude'])
  end

  def test_setter
    assert_equal(42, @attribute_dictionary['Latitude'] = 42)
  end

  def test_count
    assert_equal(7, @attribute_dictionary.count)
  end

  def test_unsupported
    assert_unsupported(@attribute_dictionary, %i[delete_key]) # no C API to remove a key
  end

  def test_empty?
    refute(@attribute_dictionary.empty?)
    assert(Sketchup.active_model.definitions['Heather'].attribute_dictionary('Fresh', true).empty?)
  end

  def test_value_types
    dictionary = Sketchup.active_model.definitions['Heather'].attribute_dictionary('Types', true)
    color = Sketchup::Color.new(1, 2, 3)
    vector = Geom::Vector3d.new(1, 2, 3)
    time = Time.at(1_700_000_000)
    dictionary['color'] = color
    dictionary['vector'] = vector
    dictionary['time'] = time
    dictionary['list'] = [1, 'two', 3.0]
    assert_equal(color, dictionary['color'])
    assert_equal(vector.to_a, dictionary['vector'].to_a)
    assert_equal(time.to_i, dictionary['time'].to_i)
    assert_equal([1, 'two', 3.0], dictionary['list'])
    assert_raises(NotImplementedError) { dictionary['nil'] = nil }
  end

  def test_each
    counter = 0
    @attribute_dictionary.each { |key, value|
      assert_instance_of(String, key)
      assert_equal(false, value.nil?)
      counter += 1
    }
    assert_equal(7, counter)
  end

  def test_each_key
    counter = 0
    @attribute_dictionary.each_key { |key|
      assert_instance_of(String, key)
      counter += 1
    }
    assert_equal(7, counter)
  end

  def test_each_pair
    counter = 0
    @attribute_dictionary.each_pair { |key, value|
      assert_instance_of(String, key)
      assert_equal(false, value.nil?)
      counter += 1
    }
    assert_equal(7, counter)
  end

  def test_keys
    assert_equal(
      ['GeoReferenceNorthAngle', 'Latitude', 'LocationSource', 'Longitude', 'ModelTranslationX', 'ModelTranslationY', 'UsesGeoReferencing'],
      @attribute_dictionary.keys
    )
  end

  def test_length
    assert_equal(7, @attribute_dictionary.length)
  end

  def test_name
    assert_equal('GeoReference', @attribute_dictionary.name)
  end

  def test_size
    assert_equal(7, @attribute_dictionary.size)
  end

  def test_count_is_alias_of_size
    assert_equal(@attribute_dictionary.size, @attribute_dictionary.count)
  end

  def test_values
    assert_equal(
      [0.1557307335588798, 40.018309, 'Manual', -105.242139, -18_871_519.616960514, -174_402_260.58333763, false],
      @attribute_dictionary.values
    )
  end

end
