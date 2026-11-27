require_relative 'helper'

class TestCamera < Minitest::Test
  def setup
    @camera = Sketchup::Camera.new([0, -10, 5], [0, 0, 0], [0, 0, 1])
  end

  def test_initialize
    assert_instance_of(Sketchup::Camera, Sketchup::Camera.new)
    camera = Sketchup::Camera.new([0, -10, 0], [0, 0, 0], [0, 0, 1], false)
    refute(camera.perspective?)
    camera = Sketchup::Camera.new([0, -10, 0], [0, 0, 0], [0, 0, 1], true, 45)
    assert_in_delta(45.0, camera.fov)
    assert_raises(ArgumentError) { Sketchup::Camera.new([0, 0, 0]) }
    assert_raises(ArgumentError) { Sketchup::Camera.new([0, 0, 0], [0, 0, 0], [0, 0, 1]) }
  end

  def test_orientation
    assert_equal(Geom::Point3d.new(0, -10, 5), @camera.eye)
    assert_equal(Geom::Point3d.new(0, 0, 0), @camera.target)
    assert_instance_of(Geom::Vector3d, @camera.up)
    assert(@camera.direction.samedirection?([0, 10, -5]))
  end

  def test_set
    assert_same(@camera, @camera.set([10, 0, 0], [0, 0, 0], [0, 0, 1]))
    assert_equal(Geom::Point3d.new(10, 0, 0), @camera.eye)
    assert(@camera.direction.samedirection?([-1, 0, 0]))
  end

  def test_axes
    camera = Sketchup::Camera.new([0, -10, 0], [0, 0, 0], [0, 0, 1])
    assert(camera.zaxis.samedirection?([0, 1, 0]))
    assert(camera.xaxis.samedirection?([1, 0, 0]))
    assert(camera.yaxis.samedirection?([0, 0, 1]))
  end

  def test_description
    assert_equal('Wide', @camera.description = 'Wide')
    assert_equal('Wide', @camera.description)
  end

  def test_fov_and_focal_length
    assert_equal(60, @camera.fov = 60)
    assert_in_delta(60.0, @camera.fov)
    @camera.image_width = 36
    assert_in_delta(36.0, @camera.image_width)
    assert_in_delta(36.0 / (2 * Math.tan(30.degrees)), @camera.focal_length)
    @camera.focal_length = 50
    assert_in_delta(50.0, @camera.focal_length, 1e-6)
    assert_raises(ArgumentError) { @camera.focal_length = 0 }
    assert_includes([true, false], @camera.fov_is_height?)
  end

  def test_aspect_ratio
    assert_equal(1.5, @camera.aspect_ratio = 1.5)
    assert_in_delta(1.5, @camera.aspect_ratio)
  end

  def test_perspective_and_height
    assert(@camera.perspective?)
    assert_equal(false, @camera.perspective = false)
    refute(@camera.perspective?)
    @camera.height = 100
    assert_in_delta(100.0, @camera.height)
  end

  def test_2d
    refute(@camera.is_2d?)
    assert_instance_of(Float, @camera.scale_2d)
    assert_instance_of(Geom::Point3d, @camera.center_2d)
  end
end
