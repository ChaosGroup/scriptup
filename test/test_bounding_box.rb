require_relative 'helper'

class TestBoundingBox < Minitest::Test
  def setup
    open_model
    @box = Geom::BoundingBox.new
    @box.add([0, 0, 0], [2, 4, 6])
  end

  def test_constants
    assert_equal(0, Geom::BoundingBox::CORNER_LEFT_FRONT_BOTTOM)
    assert_equal(1, Geom::BoundingBox::CORNER_RIGHT_FRONT_BOTTOM)
    assert_equal(2, Geom::BoundingBox::CORNER_LEFT_BACK_BOTTOM)
    assert_equal(3, Geom::BoundingBox::CORNER_RIGHT_BACK_BOTTOM)
    assert_equal(4, Geom::BoundingBox::CORNER_LEFT_FRONT_TOP)
    assert_equal(5, Geom::BoundingBox::CORNER_RIGHT_FRONT_TOP)
    assert_equal(6, Geom::BoundingBox::CORNER_LEFT_BACK_TOP)
    assert_equal(7, Geom::BoundingBox::CORNER_RIGHT_BACK_TOP)
  end

  def test_new_is_empty
    box = Geom::BoundingBox.new
    assert(box.empty?)
    refute(box.valid?)
    assert_equal(0, box.width)
    assert_raises(ArgumentError) { Geom::BoundingBox.new(1) }
  end

  def test_add
    box = Geom::BoundingBox.new
    assert_same(box, box.add(Geom::Point3d.new(1, 1, 1)))
    refute(box.empty?)
    box.add([[5, 5, 5], Geom::Point3d.new(-1, 0, 0)])
    assert_equal(Geom::Point3d.new(-1, 0, 0), box.min)
    assert_equal(Geom::Point3d.new(5, 5, 5), box.max)
    other = Geom::BoundingBox.new.add([10, 10, 10])
    box.add(other)
    assert_equal(Geom::Point3d.new(10, 10, 10), box.max)
    box.add(Geom::BoundingBox.new)
    assert_equal(Geom::Point3d.new(10, 10, 10), box.max)
    assert_raises(TypeError) { box.add('x') }
  end

  def test_min_max_center
    assert_equal(Geom::Point3d.new(0, 0, 0), @box.min)
    assert_equal(Geom::Point3d.new(2, 4, 6), @box.max)
    assert_equal(Geom::Point3d.new(1, 2, 3), @box.center)
  end

  def test_dimensions
    assert_instance_of(Length, @box.width)
    assert_equal(2, @box.width)
    assert_equal(4, @box.height)
    assert_equal(6, @box.depth)
    assert_in_delta(Math.sqrt(4 + 16 + 36), @box.diagonal.to_f)
  end

  def test_corner
    assert_equal(Geom::Point3d.new(0, 0, 0), @box.corner(Geom::BoundingBox::CORNER_LEFT_FRONT_BOTTOM))
    assert_equal(Geom::Point3d.new(2, 4, 6), @box.corner(Geom::BoundingBox::CORNER_RIGHT_BACK_TOP))
    assert_equal(Geom::Point3d.new(2, 0, 6), @box.corner(Geom::BoundingBox::CORNER_RIGHT_FRONT_TOP))
    assert_raises(IndexError) { @box.corner(8) }
  end

  def test_contains?
    assert(@box.contains?([1, 1, 1]))
    refute(@box.contains?([3, 1, 1]))
    assert(@box.contains?(Geom::BoundingBox.new.add([1, 1, 1], [2, 2, 2])))
    refute(@box.contains?(Geom::BoundingBox.new.add([1, 1, 1], [3, 3, 3])))
    refute(Geom::BoundingBox.new.contains?([0, 0, 0]))
  end

  def test_intersect
    other = Geom::BoundingBox.new.add([1, 1, 1], [10, 10, 10])
    result = @box.intersect(other)
    assert_instance_of(Geom::BoundingBox, result)
    assert_equal(Geom::Point3d.new(1, 1, 1), result.min)
    assert_equal(Geom::Point3d.new(2, 4, 6), result.max)
    assert(@box.intersect(Geom::BoundingBox.new.add([20, 20, 20])).empty?)
    assert_raises(TypeError) { @box.intersect([1, 2, 3]) }
  end

  def test_clear
    assert_same(@box, @box.clear)
    assert(@box.empty?)
  end

  def test_clone
    copy = @box.clone
    copy.clear
    refute(@box.empty?)
  end

  def test_drawingelement_bounds
    definition = Sketchup.active_model.definitions['Heather']
    bounds = definition.bounds
    assert_instance_of(Geom::BoundingBox, bounds)
  end
end
