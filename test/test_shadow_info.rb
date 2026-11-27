require_relative 'helper'

class TestShadowInfo < Minitest::Test
  UNSUPPORTED = %i[add_observer remove_observer].freeze # no C API

  def setup
    @model = open_model
    @shadow = @model.shadow_info
  end

  def test_unsupported
    assert_unsupported(@shadow, UNSUPPORTED)
  end

  def test_keys
    keys = @shadow.keys
    assert_includes(keys, 'Latitude')
    assert_equal(keys, Sketchup::ShadowInfo.keys)
    assert_equal(keys.size, @shadow.count)
    assert_equal(keys.size, @shadow.size)
    assert_equal(keys.size, @shadow.length)
  end

  def test_each
    pairs = []
    assert_nil(@shadow.each_pair { |k, v| pairs << [k, v] })
    assert_equal(@shadow.keys, pairs.map(&:first))
    keys = []
    Sketchup::ShadowInfo.each_key { |k| keys << k }
    assert_equal(@shadow.keys, keys)
  end

  def test_values
    assert_equal('Boulder (CO)', @shadow['City'])
    assert_instance_of(Float, @shadow['Latitude'])
    assert_instance_of(Time, @shadow['ShadowTime'])
    assert_instance_of(Geom::Vector3d, @shadow['SunDirection'])
    assert_includes([true, false], @shadow['DisplayShadows'])
  end

  def test_set
    assert_equal(40, @shadow['Latitude'] = 40)
    assert_in_delta(40.0, @shadow['Latitude'])
    assert_equal('Ghent', @shadow['City'] = 'Ghent')
    assert_equal('Ghent', @shadow['City'])
    @shadow['DisplayShadows'] = true
    assert(@shadow['DisplayShadows'])
    time = Time.at(1_700_000_000)
    @shadow['ShadowTime'] = time
    assert_equal(time.to_i, @shadow['ShadowTime'].to_i)
    assert_raises(ArgumentError) { @shadow['NoSuchKey'] = 1 }
  end
end
