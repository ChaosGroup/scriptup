require_relative 'helper'

class TestPoint2d < Minitest::Test
  def setup
    open_model
    @point = Geom::Point2d.new(1, 2)
  end

  def test_initialize
    assert_equal([0, 0], Geom::Point2d.new.to_a.map(&:to_f))
    assert_equal([1, 2], @point.to_a.map(&:to_f))
    assert_equal([3, 4], Geom::Point2d.new([3, 4]).to_a.map(&:to_f))
    assert_equal([1, 2], Geom::Point2d.new(@point).to_a.map(&:to_f))
    assert_raises(ArgumentError) { Geom::Point2d.new([1, 2, 3]) }
    assert_raises(TypeError) { Geom::Point2d.new('x') }
  end

  def test_components
    assert_instance_of(Length, @point.x)
    assert_equal(1, @point.x)
    assert_equal(2, @point.y)
    assert_equal(5, @point.x = 5)
    assert_equal(6, @point.y = 6)
    assert(@point == [5, 6])
  end

  def test_index
    assert_instance_of(Length, @point[0])
    assert_equal(2, @point[1])
    assert_equal(8, @point[1] = 8)
    assert_equal(8, @point.y)
    assert_raises(IndexError) { @point[2] }
  end

  def test_to_a
    @point.to_a.each { |v| assert_instance_of(Length, v) }
  end

  def test_clone_and_set!
    copy = @point.clone
    copy.x = 10
    assert_equal(1, @point.x)
    assert_same(@point, @point.set!(3, 4))
    assert(@point == [3, 4])
    @point.set!([5, 6])
    assert(@point == [5, 6])
  end

  def test_plus_and_minus
    assert_instance_of(Geom::Point2d, @point + [1, 1])
    assert(@point + Geom::Vector2d.new(1, 1) == [2, 3])
    vector = @point - Geom::Point2d.new(0, 0)
    assert_instance_of(Geom::Vector2d, vector)
    assert_equal([1.0, 2.0], vector.to_a)
    assert_instance_of(Geom::Vector2d, @point - [0, 0])
    point = @point - Geom::Vector2d.new(1, 1)
    assert_instance_of(Geom::Point2d, point)
    assert(point == [0, 1])
  end

  def test_equality
    assert(@point == Geom::Point2d.new(1, 2))
    refute(@point == [1, 3])
    refute(@point == 'x')
  end

  def test_distance
    assert_equal(5, Geom::Point2d.new(0, 0).distance([3, 4]))
  end

  def test_vector_to
    vector = @point.vector_to([4, 6])
    assert_instance_of(Geom::Vector2d, vector)
    assert_equal([3.0, 4.0], vector.to_a)
  end

  def test_offset
    assert(@point.offset([1, 0]) == [2, 2])
    assert(@point == [1, 2])
    assert(@point.offset([1, 0], 3) == [4, 2])
    assert_same(@point, @point.offset!([0, 1]))
    assert(@point == [1, 3])
    assert_raises(ArgumentError) { @point.offset([0, 0], 1) }
  end

  def test_strings
    assert_equal('Point2d(1, 2)', @point.inspect)
    assert_match(/\A\(.+, .+\)\z/, @point.to_s)
  end
end
