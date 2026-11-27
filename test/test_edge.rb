require_relative 'helper'

class TestEdge < Minitest::Test
  UNSUPPORTED = %i[explode_curve find_faces split].freeze # no C API

  def setup
    @model = open_model
    @face = add_square_face(@model.entities)
    @face_edges = @model.entities.grep(Sketchup::Edge).select { |e| e.faces.include?(@face) }
    @edge = @face_edges.first
    @line = @model.entities.add_line([0, 0, 20], [3, 4, 20])
  end

  def test_unsupported
    assert_unsupported(@edge, UNSUPPORTED)
  end

  def test_vertices
    assert_instance_of(Sketchup::Vertex, @line.start)
    assert_instance_of(Sketchup::Vertex, @line.end)
    assert_equal(Geom::Point3d.new(0, 0, 20), @line.start.position)
    assert_equal(Geom::Point3d.new(3, 4, 20), @line.end.position)
    assert_equal([@line.start, @line.end], @line.vertices)
  end

  def test_faces
    assert_equal([@face], @edge.faces)
    assert_empty(@line.faces)
  end

  def test_length
    assert_instance_of(Length, @line.length)
    assert_equal(5, @line.length)
    assert_equal(10, @line.length(Geom::Transformation.scaling(2)))
    assert_raises(TypeError) { @line.length(2) }
  end

  def test_line
    point, vector = @line.line
    assert_equal(Geom::Point3d.new(0, 0, 20), point)
    assert(vector.samedirection?([3, 4, 0]))
    assert(vector.unitvector?)
  end

  def test_other_vertex
    assert_equal(@line.end, @line.other_vertex(@line.start))
    assert_equal(@line.start, @line.other_vertex(@line.end))
    assert_raises(ArgumentError) { @line.other_vertex(@edge.start) }
  end

  def test_reversed_in?
    assert_includes([true, false], @edge.reversed_in?(@face))
    assert_raises(ArgumentError) { @line.reversed_in?(@face) }
  end

  def test_common_face
    edges = @face_edges
    assert_equal(@face, edges[0].common_face(edges[1]))
    assert_nil(@line.common_face(edges[0]))
  end

  def test_used_by?
    assert(@edge.used_by?(@face))
    assert(@line.used_by?(@line.start))
    refute(@line.used_by?(@face))
    assert_raises(TypeError) { @line.used_by?(1) }
  end

  def test_all_connected
    connected = @edge.all_connected
    assert_equal(5, connected.size) # 4 edges + 1 face
    assert_includes(connected, @face)
    refute_includes(connected, @line)
    assert_equal([@line], @line.all_connected)
  end

  def test_curve
    assert_nil(@line.curve)
    arc = @model.entities.add_arc([0, 0, 40], [1, 0, 0], [0, 0, 1], 5, 0, Math::PI, 4)
    assert_kind_of(Sketchup::ArcCurve, arc.first.curve)
    curve_edges = @model.entities.add_curve([0, 0, 60], [1, 1, 60], [2, 0, 60])
    assert_kind_of(Sketchup::Curve, curve_edges.first.curve)
  end

  def test_soft_and_smooth
    refute(@line.soft?)
    assert_equal(true, @line.soft = true)
    assert(@line.soft?)
    refute(@line.smooth?)
    @line.smooth = true
    assert(@line.smooth?)
  end
end
