require_relative 'helper'

class TestAxes < Minitest::Test
  def setup
    @model = open_model
    @axes = @model.axes
  end

  def test_class
    assert_instance_of(Sketchup::Axes, @axes)
    assert_kind_of(Sketchup::Entity, @axes)
  end

  def test_default_axes
    assert_equal(Geom::Point3d.new(0, 0, 0), @axes.origin)
    assert_equal([1.0, 0.0, 0.0], @axes.xaxis.to_a)
    assert_equal([0.0, 1.0, 0.0], @axes.yaxis.to_a)
    assert_equal([0.0, 0.0, 1.0], @axes.zaxis.to_a)
    assert(@axes.transformation.identity?)
  end

  def test_axes_and_to_a
    x, y, z = @axes.axes
    assert_equal([@axes.xaxis, @axes.yaxis, @axes.zaxis].map(&:to_a), [x, y, z].map(&:to_a))
    origin, *vectors = @axes.to_a
    assert_instance_of(Geom::Point3d, origin)
    assert_equal(3, vectors.size)
  end

  def test_sketch_plane
    plane = @axes.sketch_plane
    assert_equal(4, plane.size)
    assert(Geom::Point3d.new(5, 5, 0).on_plane?(plane))
  end

  def test_set
    assert_same(@axes, @axes.set([1, 2, 3], [0, 1, 0], [-1, 0, 0], [0, 0, 1]))
    assert_equal(Geom::Point3d.new(1, 2, 3), @axes.origin)
    assert(@axes.xaxis.samedirection?([0, 1, 0]))
    assert_equal(Geom::Point3d.new(1, 2, 3), @axes.transformation.origin)
    assert_raises(ArgumentError) { @axes.set([0, 0, 0], [1, 0, 0], [1, 0, 0], [0, 0, 1]) }
  end
end
