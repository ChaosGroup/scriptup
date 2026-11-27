require_relative 'helper'

class TestComponentInstance < Minitest::Test
  # No C API: swapping definitions, booleans, exploding, making unique, gluing and observers.
  UNSUPPORTED = %i[definition= equals? explode glued_to= intersect make_unique outer_shell show_differences
                   split subtract trim union add_observer remove_observer].freeze

  def setup
    @model = open_model
    @definition = @model.definitions['Heather']
    @instance = @model.entities.add_instance(@definition, Geom::Transformation.translation([10, 0, 0]))
  end

  def test_unsupported
    assert_unsupported(@instance, UNSUPPORTED)
  end

  def test_class
    assert_kind_of(Sketchup::Drawingelement, @instance)
    assert_equal('ComponentInstance', @instance.typename)
    assert_includes(@model.entities.grep(Sketchup::ComponentInstance), @instance)
  end

  def test_definition
    assert_equal(@definition, @instance.definition)
    assert_includes(@definition.instances, @instance)
  end

  def test_name
    assert_equal('', @instance.name)
    assert_equal('Heather 2', @instance.name = 'Heather 2')
    assert_equal('Heather 2', @instance.name)
    assert_raises(TypeError) { @instance.name = nil }
  end

  def test_guid
    assert_instance_of(String, @instance.guid)
    refute_empty(@instance.guid)
  end

  def test_locked
    refute(@instance.locked?)
    @instance.locked = true
    assert(@instance.locked?)
  end

  def test_transformation
    assert_equal(Geom::Point3d.new(10, 0, 0), @instance.transformation.origin)
    t = Geom::Transformation.translation([0, 20, 0])
    assert_equal(t, @instance.transformation = t)
    assert_equal(Geom::Point3d.new(0, 20, 0), @instance.transformation.origin)
    assert_raises(TypeError) { @instance.transformation = [1, 2] }
  end

  def test_move_and_transform
    assert_equal(true, @instance.move!(Geom::Transformation.translation([1, 0, 0])))
    assert_equal(Geom::Point3d.new(1, 0, 0), @instance.transformation.origin)
    assert_equal(true, @instance.transform!(Geom::Transformation.translation([1, 0, 0])))
    assert_equal(Geom::Point3d.new(2, 0, 0), @instance.transformation.origin)
  end

  def test_volume_and_manifold
    assert_instance_of(Float, @instance.volume)
    assert_includes([true, false], @instance.manifold?)
    assert_equal(@instance.manifold?, !@instance.volume.negative?)
  end

  def test_glued_to
    assert_nil(@instance.glued_to)
  end
end
