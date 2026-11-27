require_relative 'helper'

class TestLatLong < Minitest::Test
  UNSUPPORTED = %i[to_utm].freeze # no UTM conversion in the C API

  def setup
    @latlong = Geom::LatLong.new(40.017, 105.283)
  end

  def test_initialize
    assert_equal([0.0, 0.0], Geom::LatLong.new.to_a)
    assert_equal([40.017, 105.283], @latlong.to_a)
    assert_equal([40.017, 105.283], Geom::LatLong.new([40.017, 105.283]).to_a)
    assert_equal([40.017, 105.283], Geom::LatLong.new(@latlong).to_a)
    assert_raises(ArgumentError) { Geom::LatLong.new(91, 0) }
    assert_raises(ArgumentError) { Geom::LatLong.new(0, 181) }
    assert_raises(TypeError) { Geom::LatLong.new('x', 0) }
  end

  def test_accessors
    assert_instance_of(Float, @latlong.latitude)
    assert_equal(40.017, @latlong.latitude)
    assert_equal(105.283, @latlong.longitude)
  end

  def test_clone
    copy = @latlong.clone
    refute_same(@latlong, copy)
    assert_equal(@latlong.to_a, copy.to_a)
  end

  def test_to_s
    assert_equal('(40.017, 105.283)', @latlong.to_s)
  end

  def test_unsupported
    assert_unsupported(@latlong, UNSUPPORTED)
  end
end
