require_relative 'helper'

class TestEntitiesBuilder < Minitest::Test
  def setup
    @model = open_model
    @entities = @model.entities
  end

  def test_build
    builder = nil
    assert_nil(@entities.build { |b| builder = b })
    assert_instance_of(Sketchup::EntitiesBuilder, builder)
    refute(builder.valid?, 'only valid inside the block')
    assert_raises(RuntimeError) { builder.add_edge([0, 0, 0], [1, 0, 0]) }
    assert_raises(NoMethodError) { Sketchup::EntitiesBuilder.new }
  end

  def test_valid_and_entities
    @entities.build do |builder|
      assert(builder.valid?)
      assert_equal(@entities, builder.entities)
    end
  end

  def test_add_edge
    @entities.build do |builder|
      assert_instance_of(Sketchup::Edge, builder.add_edge([0, 0, 0], [5, 0, 0]))
      assert_instance_of(Sketchup::Edge, builder.add_edge([[0, 0, 1], [5, 0, 1]]))
      assert_raises(ArgumentError) { builder.add_edge([[0, 0, 0]]) }
    end
  end

  def test_add_edges
    @entities.build do |builder|
      assert_equal(2, builder.add_edges([0, 0, 0], [1, 0, 0], [1, 1, 0]).size)
      assert_equal(2, builder.add_edges([[0, 0, 2], [1, 0, 2], [1, 1, 2]]).size)
    end
  end

  def test_add_face
    @entities.build do |builder|
      face = builder.add_face([0, 0, 0], [10, 0, 0], [10, 10, 0], [0, 10, 0])
      assert_instance_of(Sketchup::Face, face)
      assert_instance_of(Sketchup::Face, builder.add_face([[0, 0, 5], [5, 0, 5], [5, 5, 5]]))
      assert_raises(ArgumentError) { builder.add_face([0, 0, 0], [1, 0, 0]) }
      assert_raises(ArgumentError) { builder.add_face([0, 0, 0], [1, 0, 0], [2, 0, 0]) }
    end
  end

  def test_add_face_with_holes
    @entities.build do |builder|
      outer = [[0, 0, 0], [10, 0, 0], [10, 10, 0], [0, 10, 0]]
      hole = [[3, 3, 0], [6, 3, 0], [6, 6, 0], [3, 6, 0]]
      face = builder.add_face(outer, holes: [hole])
      assert_equal(2, face.loops.size)
      assert_in_delta(91.0, face.area)
    end
  end

  def test_vertex_at
    @entities.build do |builder|
      builder.add_edge([7, 7, 7], [8, 7, 7])
      vertex = builder.vertex_at([7, 7, 7])
      assert_instance_of(Sketchup::Vertex, vertex)
      assert_equal(Geom::Point3d.new(7, 7, 7), vertex.position)
      assert_nil(builder.vertex_at([99, 99, 99]))
    end
  end
end
