require_relative 'helper'

class TestArcCurve < Minitest::Test
  def setup
    @model = open_model
    @arc = @model.entities.add_arc([1, 2, 3], [1, 0, 0], [0, 0, 1], 5, 0, Math::PI / 2, 6).first.curve
    @circle = @model.entities.add_circle([0, 0, 10], [0, 0, 1], 4).first.curve
  end

  def test_class
    assert_instance_of(Sketchup::ArcCurve, @arc)
    assert_kind_of(Sketchup::Curve, @arc)
    assert_equal(6, @arc.count_edges)
  end

  def test_center_and_radius
    assert_equal(Geom::Point3d.new(1, 2, 3), @arc.center)
    assert_instance_of(Float, @arc.radius)
    assert_in_delta(5.0, @arc.radius)
  end

  def test_angles
    assert_in_delta(0.0, @arc.start_angle)
    assert_in_delta(Math::PI / 2, @arc.end_angle)
  end

  def test_axes
    assert(@arc.normal.samedirection?([0, 0, 1]))
    assert(@arc.xaxis.parallel?([1, 0, 0]))
    assert(@arc.yaxis.perpendicular?(@arc.xaxis))
  end

  def test_plane
    plane = @arc.plane
    assert_equal(4, plane.size)
    assert(Geom::Point3d.new(9, 9, 3).on_plane?(plane))
  end

  def test_circular?
    refute(@arc.circular?)
    assert(@circle.circular?)
  end
end
