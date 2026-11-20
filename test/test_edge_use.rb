require_relative 'helper'

class TestEdgeUse < Minitest::Test
  def setup
    @model = open_model
    @face = add_square_face(@model.entities)
    @uses = @face.outer_loop.edgeuses
    @use = @uses.first
  end

  def test_edge_face_loop
    assert_instance_of(Sketchup::Edge, @use.edge)
    assert_includes(@face.edges, @use.edge)
    assert_equal(@face, @use.face)
    assert_equal(@face.outer_loop, @use.loop)
  end

  def test_next_and_previous
    assert_instance_of(Sketchup::EdgeUse, @use.next)
    assert_equal(@use, @use.next.previous)
    assert_equal(@use, @use.next.next.next.next)
  end

  def test_partners
    assert_equal([], @use.partners)
    mesh = Geom::PolygonMesh.new
    mesh.add_polygon([0, 0, 0], [10, 0, 0], [10, 10, 0])
    mesh.add_polygon([0, 0, 0], [10, 10, 0], [0, 10, 0])
    entities = @model.definitions.add('Shared').entities
    entities.fill_from_mesh(mesh, true, Geom::PolygonMesh::NO_SMOOTH_OR_HIDE)
    face = entities.grep(Sketchup::Face).first
    shared = face.outer_loop.edgeuses.find { |u| u.edge.faces.size == 2 }
    refute_nil(shared)
    assert_equal(1, shared.partners.size)
    assert_instance_of(Sketchup::EdgeUse, shared.partners.first)
  end

  def test_reversed?
    assert_includes([true, false], @use.reversed?)
  end

  def test_vertex_normals
    assert_instance_of(Geom::Vector3d, @use.start_vertex_normal)
    assert_instance_of(Geom::Vector3d, @use.end_vertex_normal)
  end
end
