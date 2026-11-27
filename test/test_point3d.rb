require_relative 'helper'

class TestPoint3d < Minitest::Test
  UNSUPPORTED = [].freeze # initialize(input_point): Sketchup::InputPoint has no C API

  def setup
    open_model
    @point = Geom::Point3d.new(1, 2, 3)
  end

  def test_initialize
    assert_equal([0, 0, 0], Geom::Point3d.new.to_a.map(&:to_f))
    assert_equal([1, 2, 3], @point.to_a.map(&:to_f))
    assert_equal([1, 2, 0], Geom::Point3d.new(1, 2).to_a.map(&:to_f))
    assert_equal([1, 2, 3], Geom::Point3d.new([1, 2, 3]).to_a.map(&:to_f))
    assert_equal([1, 2, 0], Geom::Point3d.new([1, 2]).to_a.map(&:to_f))
    assert_equal([1, 2, 3], Geom::Point3d.new(@point).to_a.map(&:to_f))
    assert_raises(TypeError) { Geom::Point3d.new('x') }
    assert_raises(ArgumentError) { Geom::Point3d.new(1, 2, 3, 4) }
  end

  def test_linear_combination
    a = Geom::Point3d.new(0, 0, 0)
    b = Geom::Point3d.new(10, 10, 10)
    assert_equal(Geom::Point3d.new(5, 5, 5), Geom::Point3d.linear_combination(0.5, a, 0.5, b))
  end

  def test_components
    assert_instance_of(Length, @point.x)
    assert_equal(1, @point.x)
    assert_equal(2, @point.y)
    assert_equal(3, @point.z)
    assert_equal(4, @point.x = 4)
    assert_equal(5, @point.y = 5)
    assert_equal(6, @point.z = 6)
    assert_equal(Geom::Point3d.new(4, 5, 6), @point)
  end

  def test_index
    assert_instance_of(Length, @point[1])
    assert_equal(2, @point[1])
    assert_equal(7, @point[2] = 7)
    assert_equal(7, @point.z)
    assert_raises(IndexError) { @point[3] }
  end

  def test_to_a
    assert_equal(3, @point.to_a.size)
    @point.to_a.each { |v| assert_instance_of(Length, v) }
  end

  def test_clone
    copy = @point.clone
    refute_same(@point, copy)
    copy.x = 9
    assert_equal(1, @point.x)
  end

  def test_set!
    assert_same(@point, @point.set!(4, 5, 6))
    assert_equal(Geom::Point3d.new(4, 5, 6), @point)
    @point.set!([7, 8, 9])
    assert_equal(Geom::Point3d.new(7, 8, 9), @point)
    @point.set!(Geom::Point3d.new(1, 1, 1))
    assert_equal(Geom::Point3d.new(1, 1, 1), @point)
  end

  def test_plus
    result = @point + Geom::Vector3d.new(1, 1, 1)
    assert_instance_of(Geom::Point3d, result)
    assert_equal(Geom::Point3d.new(2, 3, 4), result)
    assert_equal(Geom::Point3d.new(2, 3, 4), @point + [1, 1, 1])
  end

  def test_minus
    vector = @point - Geom::Point3d.new(1, 1, 1)
    assert_instance_of(Geom::Vector3d, vector)
    assert_equal([0.0, 1.0, 2.0], vector.to_a)
    assert_instance_of(Geom::Vector3d, @point - [1, 1, 1])
    point = @point - Geom::Vector3d.new(1, 1, 1)
    assert_instance_of(Geom::Point3d, point)
    assert_equal(Geom::Point3d.new(0, 1, 2), point)
  end

  def test_comparison
    assert(@point == Geom::Point3d.new(1, 2, 3))
    assert(@point == [1, 2, 3])
    refute(@point == [1, 2, 4])
    refute(@point == 'x')
    assert(Geom::Point3d.new(0, 0, 0) < @point)
    refute(@point < Geom::Point3d.new(0, 0, 0))
  end

  def test_distance
    distance = Geom::Point3d.new(0, 0, 0).distance([3, 4, 0])
    assert_instance_of(Length, distance)
    assert_equal(5, distance)
  end

  def test_vector_to
    vector = @point.vector_to([2, 2, 3])
    assert_instance_of(Geom::Vector3d, vector)
    assert_equal([1.0, 0.0, 0.0], vector.to_a)
  end

  def test_offset
    result = @point.offset(Geom::Vector3d.new(1, 0, 0))
    assert_equal(Geom::Point3d.new(2, 2, 3), result)
    assert_equal(Geom::Point3d.new(1, 2, 3), @point)
    assert_equal(Geom::Point3d.new(6, 2, 3), @point.offset([1, 0, 0], 5))
    assert_same(@point, @point.offset!([0, 0, 1]))
    assert_equal(Geom::Point3d.new(1, 2, 4), @point)
    assert_raises(ArgumentError) { @point.offset([0, 0, 0], 2) }
  end

  def test_lines
    line = [Geom::Point3d.new(0, 0, 0), Geom::Vector3d.new(1, 0, 0)]
    line2 = [Geom::Point3d.new(0, 0, 0), Geom::Point3d.new(5, 0, 0)]
    point = Geom::Point3d.new(5, 3, 4)
    assert_in_delta(5.0, point.distance_to_line(line))
    assert_in_delta(5.0, point.distance_to_line(line2))
    assert_equal(Geom::Point3d.new(5, 0, 0), point.project_to_line(line))
    refute(point.on_line?(line))
    assert(Geom::Point3d.new(-3, 0, 0).on_line?(line))
    assert_raises(ArgumentError) { point.on_line?([1, 2, 3]) }
  end

  def test_planes
    plane = [Geom::Point3d.new(0, 0, 0), Geom::Vector3d.new(0, 0, 1)]
    point = Geom::Point3d.new(1, 2, 3)
    assert_in_delta(3.0, point.distance_to_plane(plane))
    assert_in_delta(3.0, point.distance_to_plane([0, 0, 1, 0]))
    assert_equal(Geom::Point3d.new(1, 2, 0), point.project_to_plane(plane))
    refute(point.on_plane?(plane))
    assert(Geom::Point3d.new(4, 5, 0).on_plane?(plane))
    assert(Geom::Point3d.new(4, 5, 0).on_plane?([[0, 0, 0], [1, 0, 0], [0, 1, 0]]))
    assert_raises(ArgumentError) { point.on_plane?([1]) }
  end

  def test_strings
    assert_equal('Point3d(1, 2, 3)', @point.inspect)
    assert_match(/\A\(.+, .+, .+\)\z/, @point.to_s)
  end
end
