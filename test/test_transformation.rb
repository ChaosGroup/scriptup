require_relative 'helper'

class TestTransformation < Minitest::Test
  IDENTITY = [1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0].freeze

  def setup
    @origin = Geom::Point3d.new(0, 0, 0)
    @z = Geom::Vector3d.new(0, 0, 1)
  end

  def apply(transformation, point)
    transformation * Geom::Point3d.new(point)
  end

  def test_initialize
    assert(Geom::Transformation.new.identity?)
    assert_equal(IDENTITY, Geom::Transformation.new.to_a)
    assert_equal(Geom::Point3d.new(1, 2, 3), apply(Geom::Transformation.new(Geom::Point3d.new(1, 2, 3)), [0, 0, 0]))
    assert_equal(Geom::Point3d.new(1, 2, 3), apply(Geom::Transformation.new(Geom::Vector3d.new(1, 2, 3)), [0, 0, 0]))
    assert_equal(Geom::Point3d.new(2, 2, 2), apply(Geom::Transformation.new(2), [1, 1, 1]))
    copy = Geom::Transformation.new(Geom::Transformation.new(2))
    assert_equal(Geom::Transformation.new(2).to_a, copy.to_a)
    assert_equal(IDENTITY, Geom::Transformation.new(IDENTITY).to_a)
    assert_raises(TypeError) { Geom::Transformation.new('x') }
  end

  def test_initialize_axes_and_rotation
    t = Geom::Transformation.new([1, 2, 3], [0, 1, 0], [-1, 0, 0])
    assert_equal(Geom::Point3d.new(1, 2, 3), t.origin)
    assert(t.zaxis.samedirection?([0, 0, 1]))
    t = Geom::Transformation.new(@origin, @z, 90.degrees)
    assert_equal(Geom::Point3d.new(0, 1, 0), apply(t, [1, 0, 0]))
    t = Geom::Transformation.new(Geom::Vector3d.new(1, 0, 0), Geom::Vector3d.new(0, 1, 0), @z, Geom::Point3d.new(5, 0, 0))
    assert_equal(Geom::Point3d.new(5, 0, 0), t.origin)
    t = Geom::Transformation.new(Geom::Point3d.new(1, 1, 1), @z)
    assert_equal(Geom::Point3d.new(1, 1, 1), t.origin)
    assert(t.zaxis.samedirection?(@z))
  end

  def test_axes
    t = Geom::Transformation.axes([1, 2, 3], [1, 0, 0], [0, 1, 0], [0, 0, 1])
    assert_equal(Geom::Point3d.new(1, 2, 3), t.origin)
    t = Geom::Transformation.axes([0, 0, 0], [0, 1, 0], [-1, 0, 0])
    assert(t.xaxis.samedirection?([0, 1, 0]))
    assert(t.zaxis.samedirection?([0, 0, 1]))
  end

  def test_interpolate
    a = Geom::Transformation.translation([0, 0, 0])
    b = Geom::Transformation.translation([10, 0, 0])
    assert_equal(Geom::Point3d.new(5, 0, 0), Geom::Transformation.interpolate(a, b, 0.5).origin)
    assert_raises(TypeError) { Geom::Transformation.interpolate(a, 1, 0.5) }
  end

  def test_rotation
    t = Geom::Transformation.rotation(@origin, @z, 90.degrees)
    assert_equal(Geom::Point3d.new(0, 1, 0), apply(t, [1, 0, 0]))
    about = Geom::Transformation.rotation([1, 1, 0], @z, 90.degrees)
    assert_equal(Geom::Point3d.new(1, 2, 0), apply(about, [2, 1, 0]))
    assert_raises(ArgumentError) { Geom::Transformation.rotation(@origin, [0, 0, 0], 1) }
  end

  def test_scaling
    assert_equal(Geom::Point3d.new(2, 2, 2), apply(Geom::Transformation.scaling(2), [1, 1, 1]))
    assert_equal(Geom::Point3d.new(2, 3, 4), apply(Geom::Transformation.scaling(2, 3, 4), [1, 1, 1]))
    assert_equal(Geom::Point3d.new(3, 3, 3), apply(Geom::Transformation.scaling([1, 1, 1], 2), [2, 2, 2]))
    assert_equal(Geom::Point3d.new(3, 1, 1), apply(Geom::Transformation.scaling([1, 1, 1], 2, 1, 1), [2, 1, 1]))
    assert_raises(ArgumentError) { Geom::Transformation.scaling(1, 2) }
  end

  def test_translation
    assert_equal(Geom::Point3d.new(1, 2, 3), Geom::Transformation.translation(Geom::Vector3d.new(1, 2, 3)).origin)
    assert_equal(Geom::Point3d.new(1, 2, 3), Geom::Transformation.translation(Geom::Point3d.new(1, 2, 3)).origin)
  end

  def test_multiply
    t = Geom::Transformation.translation([1, 0, 0])
    assert_instance_of(Geom::Point3d, t * Geom::Point3d.new(0, 0, 0))
    vector = t * Geom::Vector3d.new(1, 0, 0)
    assert_instance_of(Geom::Vector3d, vector)
    assert_equal([1.0, 0.0, 0.0], vector.to_a) # translation doesn't move vectors
    combined = t * t
    assert_instance_of(Geom::Transformation, combined)
    assert_equal(Geom::Point3d.new(2, 0, 0), combined.origin)
    assert_equal([1.0, 0.0, 0.0], t * [0, 0, 0])
    plane = Geom::Transformation.translation([0, 0, 5]) * [Geom::Point3d.new(0, 0, 0), Geom::Vector3d.new(0, 0, 1)]
    assert_equal(4, plane.size)
    assert_in_delta(-5.0, plane[3])
    assert_in_delta(-5.0, (Geom::Transformation.translation([0, 0, 5]) * [0, 0, 1, 0])[3])
  end

  def test_inverse
    t = Geom::Transformation.translation([1, 2, 3])
    assert_equal(Geom::Point3d.new(-1, -2, -3), t.inverse.origin)
    assert_equal(Geom::Point3d.new(1, 2, 3), t.origin)
    assert_same(t, t.invert!)
    assert_equal(Geom::Point3d.new(-1, -2, -3), t.origin)
    assert((Geom::Transformation.translation([1, 2, 3]) * Geom::Transformation.translation([1, 2, 3]).inverse).identity?)
  end

  def test_axes_accessors
    t = Geom::Transformation.new
    assert_instance_of(Geom::Vector3d, t.xaxis)
    assert_equal([1.0, 0.0, 0.0], t.xaxis.to_a)
    assert_equal([0.0, 1.0, 0.0], t.yaxis.to_a)
    assert_equal([0.0, 0.0, 1.0], t.zaxis.to_a)
    assert_instance_of(Geom::Point3d, t.origin)
  end

  def test_set!
    t = Geom::Transformation.new
    assert_same(t, t.set!(Geom::Point3d.new(1, 2, 3)))
    assert_equal(Geom::Point3d.new(1, 2, 3), t.origin)
    t.set!(2)
    assert_equal(Geom::Point3d.new(2, 2, 2), apply(t, [1, 1, 1]))
    t.set!(IDENTITY)
    assert(t.identity?)
  end

  def test_clone
    t = Geom::Transformation.translation([1, 0, 0])
    copy = t.clone
    copy.invert!
    assert_equal(Geom::Point3d.new(1, 0, 0), t.origin)
  end

  def test_point_and_vector_transform
    t = Geom::Transformation.translation([1, 0, 0])
    point = Geom::Point3d.new(0, 0, 0)
    assert_equal(Geom::Point3d.new(1, 0, 0), point.transform(t))
    assert_equal(Geom::Point3d.new(0, 0, 0), point)
    assert_same(point, point.transform!(t))
    assert_equal(Geom::Point3d.new(1, 0, 0), point)
    rotate = Geom::Transformation.rotation(@origin, @z, 90.degrees)
    vector = Geom::Vector3d.new(1, 0, 0)
    assert(vector.transform(rotate).samedirection?([0, 1, 0]))
    assert_same(vector, vector.transform!(rotate))
    assert(vector.samedirection?([0, 1, 0]))
    assert_raises(TypeError) { point.transform(1) }
  end
end
