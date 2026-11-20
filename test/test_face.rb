require_relative 'helper'

class TestFace < Minitest::Test
  # No C API: extrusion, follow me, setting a projection; get_UVHelper arrives with UVHelper.
  UNSUPPORTED = %i[pushpull followme set_texture_projection].freeze

  def setup
    @model = open_model
    @entities = @model.entities
    @face = add_square_face(@entities, 10, 5)
  end

  def test_unsupported
    assert_unsupported(@face, UNSUPPORTED)
  end

  def test_constants
    values = %i[PointUnknown PointInside PointOnVertex PointOnEdge PointOnFace PointOutside PointNotOnPlane].map { |c| Sketchup::Face.const_get(c) }
    assert_equal(values.uniq.size, values.size)
    assert_equal(0, Sketchup::Face::PointUnknown)
  end

  def test_edges_vertices_loops
    assert_equal(4, @face.edges.size)
    @face.edges.each { |e| assert_instance_of(Sketchup::Edge, e) }
    assert_equal(4, @face.vertices.size)
    @face.vertices.each { |v| assert_instance_of(Sketchup::Vertex, v) }
    assert_instance_of(Sketchup::Loop, @face.outer_loop)
    assert_equal([@face.outer_loop], @face.loops)
  end

  def test_normal_and_plane
    assert_instance_of(Geom::Vector3d, @face.normal)
    assert(@face.normal.parallel?([0, 0, 1]))
    plane = @face.plane
    assert_equal(4, plane.size)
    assert(Geom::Point3d.new(3, 3, 5).on_plane?(plane))
  end

  def test_area
    assert_instance_of(Float, @face.area)
    assert_in_delta(100.0, @face.area)
    assert_in_delta(400.0, @face.area(Geom::Transformation.scaling(2)))
    assert_raises(TypeError) { @face.area(1) }
  end

  def test_materials
    assert_nil(@face.material)
    assert_nil(@face.back_material)
    shirt = @model.materials['Heather_Shirt']
    assert_equal(shirt, @face.material = shirt)
    assert_equal(shirt, @face.material)
    assert_equal('Heather_Jeans', @face.back_material = 'Heather_Jeans')
    assert_equal('Heather_Jeans', @face.back_material.name)
    @face.material = nil
    assert_nil(@face.material)
    @face.back_material = Sketchup::Color.new(1, 2, 3)
    assert_equal(Sketchup::Color.new(1, 2, 3), @face.back_material.color)
    assert_raises(TypeError) { @face.material = 1 }
  end

  def test_reverse!
    normal = @face.normal
    assert_same(@face, @face.reverse!)
    assert(@face.normal.samedirection?(normal.reverse))
  end

  def test_coplanar_with?
    other = @entities.add_face([20, 0, 5], [30, 0, 5], [30, 10, 5])
    assert(@face.coplanar_with?(other))
    refute(@face.coplanar_with?(@entities.add_face([20, 0, 9], [30, 0, 9], [30, 10, 9])))
    assert_raises(TypeError) { @face.coplanar_with?(1) }
  end

  def test_all_connected
    connected = @face.all_connected
    assert_includes(connected, @face)
    assert_equal(5, connected.size)
  end

  def test_get_glued_instances
    assert_equal([], @face.get_glued_instances)
  end

  def test_texture_queries
    refute(@face.texture_positioned?(true))
    refute(@face.texture_projected?(true))
    assert_nil(@face.get_texture_projection(true))
    assert(@face.clear_texture_position(true))
    assert(@face.clear_texture_projection(false))
    assert_nil(@face.uv_tile_at(Geom::Point3d.new(0, 0, 5), true))
  end

  def test_position_material
    material = @model.materials['Heather_Shirt']
    material.texture = File.join(TEST_RESOURCES, 'texture_map.png')
    mapping = [Geom::Point3d.new(0, 0, 5), Geom::Point3d.new(0, 0, 1), Geom::Point3d.new(10, 0, 5), Geom::Point3d.new(1, 0, 1)]
    assert_same(@face, @face.position_material(material, mapping, true))
    assert(@face.texture_positioned?(true))
    tile = @face.uv_tile_at(Geom::Point3d.new(1, 1, 5), true)
    assert_equal(8, tile.size)
    assert_raises(ArgumentError) { @face.position_material(material, [Geom::Point3d.new(0, 0, 5)], true) }
  end

  def test_mesh
    mesh = @face.mesh
    assert_instance_of(Geom::PolygonMesh, mesh)
    assert_equal(4, mesh.count_points)
    assert_equal(2, mesh.count_polygons)
    assert_instance_of(Geom::Vector3d, mesh.normal_at(1))
    assert(mesh.normal_at(1).parallel?([0, 0, 1]))
  end

  def test_classify_point
    assert_equal(Sketchup::Face::PointInside, @face.classify_point([5, 5, 5]))
    assert_equal(Sketchup::Face::PointOnVertex, @face.classify_point([0, 0, 5]))
    assert_equal(Sketchup::Face::PointOnEdge, @face.classify_point([5, 0, 5]))
    assert_equal(Sketchup::Face::PointOutside, @face.classify_point([15, 5, 5]))
    assert_equal(Sketchup::Face::PointNotOnPlane, @face.classify_point([5, 5, 9]))
  end
end
