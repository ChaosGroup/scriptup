require_relative 'helper'

class TestVertex < Minitest::Test
  def setup
    @model = open_model
    @face = add_square_face(@model.entities)
    @face_edges = @model.entities.grep(Sketchup::Edge).select { |e| e.faces.include?(@face) }
    @edge = @model.entities.add_line([0, 0, 20], [5, 0, 20])
    @vertex = @edge.start
  end

  def test_class
    assert_instance_of(Sketchup::Vertex, @vertex)
    assert_kind_of(Sketchup::Entity, @vertex)
  end

  def test_position
    assert_instance_of(Geom::Point3d, @vertex.position)
    assert_equal(Geom::Point3d.new(0, 0, 20), @vertex.position)
  end

  def test_edges
    assert_equal([@edge], @vertex.edges)
  end

  def test_faces_and_loops
    corner = @face_edges.first.start
    assert_equal([@face], corner.faces)
    assert_equal(1, corner.loops.size)
    assert_instance_of(Sketchup::Loop, corner.loops.first)
    assert_empty(@vertex.faces)
  end

  def test_common_edge
    assert_equal(@edge, @vertex.common_edge(@edge.end))
    other = @model.entities.add_line([50, 50, 50], [60, 50, 50])
    assert_nil(@vertex.common_edge(other.start))
    assert_raises(TypeError) { @vertex.common_edge(@edge) }
  end

  def test_used_by?
    assert(@vertex.used_by?(@edge))
    refute(@vertex.used_by?(@face))
    assert(@face_edges.first.start.used_by?(@face))
    assert_raises(TypeError) { @vertex.used_by?(1) }
  end

  def test_curve_interior?
    assert_nil(@vertex.curve_interior?)
    arc = @model.entities.add_arc([0, 0, 40], [1, 0, 0], [0, 0, 1], 5, 0, Math::PI, 4)
    interior = arc.first.end
    assert_kind_of(Sketchup::ArcCurve, interior.curve_interior?)
    assert_nil(arc.first.start.curve_interior?)
  end

  def test_point3d_like
    assert_equal(Geom::Point3d.new(5, 0, 20), @edge.end.position)
    edge = @model.entities.add_line(@vertex, [0, 9, 20])
    assert_equal(@vertex.position, edge.start.position)
  end
end
