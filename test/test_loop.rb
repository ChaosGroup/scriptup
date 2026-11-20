require_relative 'helper'

class TestLoop < Minitest::Test
  def setup
    @model = open_model
    @face = add_square_face(@model.entities)
    @loop = @face.outer_loop
  end

  def test_class
    assert_instance_of(Sketchup::Loop, @loop)
    assert_kind_of(Sketchup::Entity, @loop)
  end

  def test_vertices_and_edges
    assert_equal(4, @loop.vertices.size)
    @loop.vertices.each { |v| assert_instance_of(Sketchup::Vertex, v) }
    assert_equal(4, @loop.edges.size)
    assert_equal(@face.edges.sort_by(&:entityID), @loop.edges.sort_by(&:entityID))
  end

  def test_edgeuses
    uses = @loop.edgeuses
    assert_equal(4, uses.size)
    uses.each { |u| assert_instance_of(Sketchup::EdgeUse, u) }
  end

  def test_face
    assert_equal(@face, @loop.face)
  end

  def test_predicates
    assert(@loop.outer?)
    assert(@loop.convex?)
    l_shape = @model.entities.add_face([0, 0, 9], [4, 0, 9], [4, 1, 9], [1, 1, 9], [1, 4, 9], [0, 4, 9])
    refute(l_shape.outer_loop.convex?)
  end
end
