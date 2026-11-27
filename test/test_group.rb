require_relative 'helper'

class TestGroup < Minitest::Test
  # No C API: boolean operations, copying, exploding, converting, gluing and observers.
  UNSUPPORTED = %i[copy equals? explode glued_to= intersect make_unique outer_shell show_differences split
                   subtract to_component trim union add_observer remove_observer].freeze

  def setup
    @model = open_model
    @group = @model.entities.add_group
    add_square_face(@group.entities)
  end

  def test_unsupported
    assert_unsupported(@group, UNSUPPORTED)
  end

  def test_class
    assert_kind_of(Sketchup::Drawingelement, @group)
    assert_equal('Group', @group.typename)
  end

  def test_entities
    assert_instance_of(Sketchup::Entities, @group.entities)
    assert_equal(1, @group.entities.grep(Sketchup::Face).size)
    assert_equal(@group.definition, @group.entities.parent)
  end

  def test_definition
    definition = @group.definition
    assert_instance_of(Sketchup::ComponentDefinition, definition)
    assert(definition.group?)
  end

  def test_name
    assert_equal('', @group.name)
    assert_equal('Box', @group.name = 'Box')
    assert_equal('Box', @group.name)
    assert_raises(TypeError) { @group.name = 1 }
  end

  def test_description
    assert_equal('Notes', @group.description = 'Notes')
    assert_equal('Notes', @group.description)
  end

  def test_guid
    assert_instance_of(String, @group.guid)
    refute_empty(@group.guid)
  end

  def test_locked
    refute(@group.locked?)
    assert_equal(true, @group.locked = true)
    assert(@group.locked?)
  end

  def test_transformation
    assert(@group.transformation.identity?)
    t = Geom::Transformation.translation([5, 0, 0])
    assert_equal(t, @group.transformation = t)
    assert_equal(Geom::Point3d.new(5, 0, 0), @group.transformation.origin)
    assert_raises(TypeError) { @group.transformation = 1 }
  end

  def test_move_and_transform
    assert_same(@group, @group.move!(Geom::Transformation.translation([1, 0, 0])))
    assert_equal(Geom::Point3d.new(1, 0, 0), @group.transformation.origin)
    assert_same(@group, @group.transform!(Geom::Transformation.translation([1, 0, 0])))
    assert_equal(Geom::Point3d.new(2, 0, 0), @group.transformation.origin)
  end

  def test_volume_and_manifold
    assert_instance_of(Float, @group.volume)
    refute(@group.manifold?, 'a single face is not a solid')
    assert(@group.volume.negative?)
  end

  def test_local_bounds
    bounds = @group.local_bounds
    assert_instance_of(Geom::BoundingBox, bounds)
    assert_equal(10, bounds.width)
  end

  def test_glued_to
    assert_nil(@group.glued_to)
  end
end
