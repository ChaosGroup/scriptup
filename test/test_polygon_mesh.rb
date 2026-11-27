require_relative 'helper'

class TestPolygonMesh < Minitest::Test
  def setup
    @mesh = Geom::PolygonMesh.new
    @p1 = Geom::Point3d.new(0, 0, 0)
    @p2 = Geom::Point3d.new(10, 0, 0)
    @p3 = Geom::Point3d.new(10, 10, 0)
    @p4 = Geom::Point3d.new(0, 10, 0)
  end

  def test_constants
    assert_equal(0, Geom::PolygonMesh::NO_SMOOTH_OR_HIDE)
    assert_equal(1, Geom::PolygonMesh::HIDE_BASED_ON_INDEX)
    assert_equal(2, Geom::PolygonMesh::SOFTEN_BASED_ON_INDEX)
    assert_equal(4, Geom::PolygonMesh::AUTO_SOFTEN)
    assert_equal(8, Geom::PolygonMesh::SMOOTH_SOFT_EDGES)
    assert_equal(0, Geom::PolygonMesh::MESH_POINTS)
    assert_equal(1, Geom::PolygonMesh::MESH_UVQ_FRONT)
    assert_equal(3, Geom::PolygonMesh::MESH_UVQ_BACK)
    assert_equal(4, Geom::PolygonMesh::MESH_NORMALS)
  end

  def test_initialize
    assert_equal(0, Geom::PolygonMesh.new(4).count_points)
    assert_equal(0, Geom::PolygonMesh.new(4, 2).count_polygons)
    assert_raises(TypeError) { Geom::PolygonMesh.new('x') }
  end

  def test_add_point
    assert_equal(1, @mesh.add_point(@p1))
    assert_equal(2, @mesh.add_point(@p2))
    assert_equal(1, @mesh.add_point(Geom::Point3d.new(0, 0, 0)), 'duplicate points share an index')
    assert_equal(2, @mesh.count_points)
  end

  def test_add_polygon_by_index
    [@p1, @p2, @p3, @p4].each { |p| @mesh.add_point(p) }
    assert_equal(1, @mesh.add_polygon(1, 2, 3))
    assert_equal(2, @mesh.add_polygon([1, 3, -4]))
    assert_equal(2, @mesh.count_polygons)
    assert_equal([1, 3, -4], @mesh.polygon_at(2))
    assert_raises(ArgumentError) { @mesh.add_polygon(1, 2, 9) }
    assert_raises(ArgumentError) { @mesh.add_polygon(1, 2) }
  end

  def test_add_polygon_by_points
    assert_equal(1, @mesh.add_polygon(@p1, @p2, @p3))
    assert_equal(2, @mesh.add_polygon([@p1, @p3, @p4]))
    assert_equal(4, @mesh.count_points)
    assert_equal([[1, 2, 3], [1, 3, 4]], @mesh.polygons)
  end

  def test_point_accessors
    @mesh.add_polygon(@p1, @p2, @p3)
    assert_equal(@p2, @mesh.point_at(2))
    assert_nil(@mesh.point_at(0))
    assert_nil(@mesh.point_at(4))
    assert_equal(3, @mesh.point_index(@p3))
    assert_equal(0, @mesh.point_index(@p4))
    assert_equal([@p1, @p2, @p3], @mesh.points)
    assert_same(@mesh, @mesh.set_point(1, [5, 5, 5]))
    assert_equal(Geom::Point3d.new(5, 5, 5), @mesh.point_at(1))
    assert_raises(IndexError) { @mesh.set_point(9, @p1) }
  end

  def test_polygon_accessors
    @mesh.add_polygon(@p1, @p2, @p3)
    assert_equal([1, 2, 3], @mesh.polygon_at(1))
    assert_nil(@mesh.polygon_at(2))
    assert_equal([@p1, @p2, @p3], @mesh.polygon_points_at(1))
    assert_nil(@mesh.polygon_points_at(5))
    @mesh.polygons.first << 99
    assert_equal([1, 2, 3], @mesh.polygon_at(1), 'returned arrays are copies')
  end

  def test_uvs
    @mesh.add_polygon(@p1, @p2, @p3)
    assert_nil(@mesh.set_uv(1, [0, 0, 1], true))
    @mesh.set_uv(2, [1, 0, 1], true)
    @mesh.set_uv(1, [0.5, 0.5, 1], false)
    assert_equal(Geom::Point3d.new(1, 0, 1), @mesh.uv_at(2, true))
    assert_equal(Geom::Point3d.new(0.5, 0.5, 1), @mesh.uv_at(1, false))
    assert_nil(@mesh.uv_at(3, true))
    assert_equal(2, @mesh.uvs(true).size)
    assert_raises(IndexError) { @mesh.set_uv(9, [0, 0, 0], true) }
  end

  def test_normal_at
    @mesh.add_polygon(@p1, @p2, @p3)
    assert_nil(@mesh.normal_at(1), 'only meshes from faces have normals')
  end

  def test_transform!
    @mesh.add_polygon(@p1, @p2, @p3)
    assert_same(@mesh, @mesh.transform!(Geom::Transformation.translation([0, 0, 5])))
    assert_equal(Geom::Point3d.new(10, 0, 5), @mesh.point_at(2))
    assert_raises(TypeError) { @mesh.transform!(1) }
  end
end
