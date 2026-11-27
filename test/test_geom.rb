require_relative 'helper'

class TestGeom < Minitest::Test
  UNSUPPORTED = %i[tesselate].freeze # no standalone triangulation in the C API

  def setup
    @x_line = [Geom::Point3d.new(0, 0, 0), Geom::Vector3d.new(1, 0, 0)]
    @y_line = [Geom::Point3d.new(5, -5, 0), Geom::Vector3d.new(0, 1, 0)]
  end

  def test_closest_points
    points = Geom.closest_points(@x_line, [Geom::Point3d.new(5, 0, 3), Geom::Vector3d.new(0, 1, 0)])
    assert_equal(2, points.size)
    assert_equal(Geom::Point3d.new(5, 0, 0), points[0])
    assert_equal(Geom::Point3d.new(5, 0, 3), points[1])
    parallel = Geom.closest_points(@x_line, [Geom::Point3d.new(0, 2, 0), Geom::Vector3d.new(1, 0, 0)])
    assert_in_delta(2.0, parallel[0].distance(parallel[1]).to_f)
  end

  def test_intersect_line_line
    assert_equal(Geom::Point3d.new(5, 0, 0), Geom.intersect_line_line(@x_line, @y_line))
    assert_nil(Geom.intersect_line_line(@x_line, [Geom::Point3d.new(0, 1, 0), Geom::Vector3d.new(1, 0, 0)]))
    assert_nil(Geom.intersect_line_line(@x_line, [Geom::Point3d.new(5, 0, 3), Geom::Vector3d.new(0, 1, 0)]))
    assert_equal(Geom::Point3d.new(5, 0, 0), Geom.intersect_line_line(@x_line, [Geom::Point3d.new(5, -5, 0), Geom::Point3d.new(5, 5, 0)]))
  end

  def test_intersect_line_plane
    plane = [Geom::Point3d.new(0, 0, 5), Geom::Vector3d.new(0, 0, 1)]
    line = [Geom::Point3d.new(1, 2, 0), Geom::Vector3d.new(0, 0, 1)]
    assert_equal(Geom::Point3d.new(1, 2, 5), Geom.intersect_line_plane(line, plane))
    assert_equal(Geom::Point3d.new(1, 2, 5), Geom.intersect_line_plane(line, [0, 0, 1, -5]))
    assert_nil(Geom.intersect_line_plane(@x_line, plane))
  end

  def test_intersect_plane_plane
    xy = [Geom::Point3d.new(0, 0, 2), Geom::Vector3d.new(0, 0, 1)]
    xz = [Geom::Point3d.new(0, 3, 0), Geom::Vector3d.new(0, 1, 0)]
    point, vector = Geom.intersect_plane_plane(xy, xz)
    assert_instance_of(Geom::Point3d, point)
    assert_instance_of(Geom::Vector3d, vector)
    assert(vector.parallel?([1, 0, 0]))
    assert_in_delta(2.0, point.z.to_f)
    assert_in_delta(3.0, point.y.to_f)
    assert_nil(Geom.intersect_plane_plane(xy, [Geom::Point3d.new(0, 0, 9), Geom::Vector3d.new(0, 0, 1)]))
  end

  def test_linear_combination
    point = Geom.linear_combination(0.5, Geom::Point3d.new(0, 0, 0), 0.5, Geom::Point3d.new(10, 0, 0))
    assert_instance_of(Geom::Point3d, point)
    assert_equal(Geom::Point3d.new(5, 0, 0), point)
    vector = Geom.linear_combination(1, Geom::Vector3d.new(1, 0, 0), 2, Geom::Vector3d.new(0, 1, 0))
    assert_instance_of(Geom::Vector3d, vector)
    assert_equal([1.0, 2.0, 0.0], vector.to_a)
  end

  def test_fit_plane_to_points
    plane = Geom.fit_plane_to_points(Geom::Point3d.new(0, 0, 1), Geom::Point3d.new(1, 0, 1), Geom::Point3d.new(0, 1, 1))
    assert_equal(4, plane.size)
    assert_in_delta(1.0, plane[2].abs)
    assert_in_delta(1.0, plane[3].abs)
    points = [[0, 0, 2], [4, 0, 2], [4, 4, 2], [0, 4, 2]].map { |p| Geom::Point3d.new(p) }
    a, b, c, d = Geom.fit_plane_to_points(points)
    assert(Geom::Point3d.new(2, 2, 2).on_plane?([a, b, c, d]))
    assert_raises(ArgumentError) { Geom.fit_plane_to_points([Geom::Point3d.new(0, 0, 0)]) }
  end

  def test_point_in_polygon_2D
    square = [[0, 0, 0], [10, 0, 0], [10, 10, 0], [0, 10, 0]].map { |p| Geom::Point3d.new(p) }
    assert(Geom.point_in_polygon_2D(Geom::Point3d.new(5, 5, 0), square, true))
    refute(Geom.point_in_polygon_2D(Geom::Point3d.new(15, 5, 0), square, true))
    assert(Geom.point_in_polygon_2D(Geom::Point3d.new(10, 5, 0), square, true))
    refute(Geom.point_in_polygon_2D(Geom::Point3d.new(10, 5, 0), square, false))
    assert(Geom.point_in_polygon_2D(Geom::Point3d.new(5, 5, 99), square, false), 'z is ignored')
  end

  def test_unsupported
    assert_unsupported(Geom, UNSUPPORTED)
  end
end
