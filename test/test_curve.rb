require_relative 'helper'

class TestCurve < Minitest::Test
  def setup
    @model = open_model
    @edges = @model.entities.add_curve([0, 0, 0], [3, 4, 0], [6, 0, 0], [9, 4, 0])
    @curve = @edges.first.curve
  end

  def test_class
    assert_instance_of(Sketchup::Curve, @curve)
    assert_kind_of(Sketchup::Entity, @curve)
  end

  def test_edges
    assert_equal(3, @curve.count_edges)
    assert_equal(@edges, @curve.edges)
    assert_equal(@edges.first, @curve.first_edge)
    assert_equal(@edges.last, @curve.last_edge)
  end

  def test_each_edge
    seen = []
    assert_nil(@curve.each_edge { |e| seen << e })
    assert_equal(@edges, seen)
  end

  def test_length
    assert_instance_of(Length, @curve.length)
    assert_equal(15, @curve.length)
  end

  def test_vertices
    vertices = @curve.vertices
    assert_equal(4, vertices.size)
    assert_equal(Geom::Point3d.new(0, 0, 0), vertices.first.position)
    assert_equal(Geom::Point3d.new(9, 4, 0), vertices.last.position)
  end

  def test_is_polygon?
    refute(@curve.is_polygon?)
  end

  def test_move_vertices
    points = @curve.vertices.map { |v| v.position.offset([0, 0, 1]) }
    assert(@curve.move_vertices(points))
    assert_equal(Geom::Point3d.new(0, 0, 1), @curve.vertices.first.position)
    assert_raises(ArgumentError) { @curve.move_vertices([[0, 0, 0]]) }
  end
end
