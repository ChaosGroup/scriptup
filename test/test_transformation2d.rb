require_relative 'helper'

class TestTransformation2d < Minitest::Test
  IDENTITY = [1.0, 0.0, 0.0, 1.0, 0.0, 0.0].freeze

  def test_initialize
    assert(Geom::Transformation2d.new.identity?)
    assert_equal(IDENTITY, Geom::Transformation2d.new.to_a)
    t = Geom::Transformation2d.new([1.0, 0.0, 0.0, 1.0, 1.0, 1.0])
    refute(t.identity?)
    assert_equal(t.to_a, Geom::Transformation2d.new(t).to_a)
    assert_raises(ArgumentError) { Geom::Transformation2d.new([1, 2]) }
  end

  def test_equality
    assert(Geom::Transformation2d.new([1.0, 0.0, 0.0, 1.0, 1.0, 1.0]) == Geom::Transformation2d.translation([1, 1]))
    refute(Geom::Transformation2d.new == Geom::Transformation2d.translation([1, 1]))
    refute(Geom::Transformation2d.new == 'x')
  end

  def test_translation
    assert(Geom::Transformation2d.translation(Geom::Vector2d.new(2, 3)) * Geom::Point2d.new(0, 0) == [2, 3])
    assert(Geom::Transformation2d.translation(Geom::Point2d.new(2, 3)) * Geom::Point2d.new(1, 1) == [3, 4])
  end

  def test_rotation
    t = Geom::Transformation2d.rotation(Geom::Point2d.new(0, 0), 90.degrees)
    assert(t * Geom::Point2d.new(1, 0) == [0, 1])
    about = Geom::Transformation2d.rotation(Geom::Point2d.new(1, 1), 90.degrees)
    assert(about * Geom::Point2d.new(2, 1) == [1, 2])
  end

  def test_scaling
    assert(Geom::Transformation2d.scaling(2) * Geom::Point2d.new(1, 1) == [2, 2])
    assert(Geom::Transformation2d.scaling(2, 3) * Geom::Point2d.new(1, 1) == [2, 3])
    assert(Geom::Transformation2d.scaling(Geom::Point2d.new(1, 1), 2) * Geom::Point2d.new(2, 2) == [3, 3])
    assert(Geom::Transformation2d.scaling(Geom::Point2d.new(1, 1), 2, 1) * Geom::Point2d.new(2, 2) == [3, 2])
    assert_raises(ArgumentError) { Geom::Transformation2d.scaling(1, 2, 3) }
  end

  def test_multiply
    t = Geom::Transformation2d.translation([1, 0])
    assert_instance_of(Geom::Point2d, t * Geom::Point2d.new(0, 0))
    vector = t * Geom::Vector2d.new(1, 0)
    assert_instance_of(Geom::Vector2d, vector)
    assert_equal([1.0, 0.0], vector.to_a)
    assert_instance_of(Geom::Transformation2d, t * t)
    assert(t * t * Geom::Point2d.new(0, 0) == [2, 0])
    assert_equal([1.0, 0.0], t * [0, 0])
  end

  def test_inverse
    t = Geom::Transformation2d.translation([1, 2])
    assert(t.inverse * Geom::Point2d.new(0, 0) == [-1, -2])
    assert(t * Geom::Point2d.new(0, 0) == [1, 2])
    assert_same(t, t.invert!)
    assert(t * Geom::Point2d.new(0, 0) == [-1, -2])
  end

  def test_set_and_clone
    t = Geom::Transformation2d.new
    assert_same(t, t.set!([1.0, 0.0, 0.0, 1.0, 5.0, 0.0]))
    assert(t * Geom::Point2d.new(0, 0) == [5, 0])
    copy = t.clone
    copy.invert!
    assert(t * Geom::Point2d.new(0, 0) == [5, 0])
    t.set!(Geom::Transformation2d.new)
    assert(t.identity?)
  end

  def test_point_and_vector_transform
    t = Geom::Transformation2d.translation([1, 0])
    point = Geom::Point2d.new(0, 0)
    assert(point.transform(t) == [1, 0])
    assert(point == [0, 0])
    assert_same(point, point.transform!(t))
    assert(point == [1, 0])
    rotate = Geom::Transformation2d.rotation(Geom::Point2d.new(0, 0), 90.degrees)
    vector = Geom::Vector2d.new(1, 0)
    assert(vector.transform(rotate).same_direction?([0, 1]))
    assert_same(vector, vector.transform!(rotate))
    assert(vector.same_direction?([0, 1]))
    assert_raises(TypeError) { point.transform(Geom::Transformation.new) }
  end
end
