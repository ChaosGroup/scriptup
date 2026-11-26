require_relative 'helper'

class TestComponentDefinition < Minitest::Test
  # No C API: thumbnails, bounds invalidation, per-definition classification values and observers.
  # add_classification arrives with Sketchup::Classifications.
  UNSUPPORTED = %i[refresh_thumbnail save_thumbnail thumbnail_camera thumbnail_camera= invalidate_bounds insertion_point=
                   get_classification_value set_classification_value remove_classification
                   add_observer remove_observer].freeze

  def setup
    Sketchup.open_file("#{ENV['TEST_RESOURCES']}/Untitled.skp")
    @definition = Sketchup.active_model.definitions.find { |d| d.name == 'Heather' }
  end

  def test_object
    assert_kind_of(Sketchup::Drawingelement, @definition)
  end

  def test_spaceship_operator
    assert_equal(0, @definition <=> @definition)
    box_definition = Sketchup.active_model.definitions.find { |d| d.name == 'Group#1' }
    assert_equal(1, @definition <=> box_definition)
    assert_equal(-1, box_definition <=> @definition)
    assert_nil(@definition <=> 42)
  end

  def test_equals
    assert(@definition == @definition)
    assert(@definition == Sketchup.active_model.definitions.find { |d| d.name == 'Heather' })
    box_definition = Sketchup.active_model.definitions.find { |d| d.name == 'Group#1' }
    refute(@definition == box_definition)
    refute(@definition == Object.new)
  end


  def test_behavior
    assert_instance_of(Sketchup::Behavior, @definition.behavior)
  end

  def test_count_instances
    assert_equal(2, @definition.count_instances)
  end

  def test_count_used_instances
    assert_equal(2, @definition.count_used_instances)
  end

  def test_description
    assert_equal('Heather is a software engineer at SketchUp. In her free time, she’s an extreme gardener and cheesemaker who relaxes by traveling mountains by hand and foot, ice axe, skis and bicycle. Heather is a proud cat lady and animal advocate.', @definition.description)
    assert_equal('Test Description', @definition.description = 'Test Description')
    assert_equal('Test Description', @definition.description)
    assert_raises(TypeError) {
      @definition.description = Object.new
    }
    assert_raises(TypeError) {
      @definition.description = 1
    }
  end

  def test_entities
    assert_instance_of(Sketchup::Entities, @definition.entities)
  end

  def test_group?
    refute(@definition.group?)
    box_definition = Sketchup.active_model.definitions.find { |d| d.name == 'Group#1' }
    assert(box_definition.group?)
  end

  def test_guid
    assert_equal('0BRlipAQn5NfsR9CLTgK7v', @definition.guid)
  end

  def test_image?
    refute(@definition.image?)
  end

  def test_instances
    assert_instance_of(Array, @definition.instances)
  end

  def test_internal?
    refute(@definition.internal?)
  end

  def test_live_component?
    refute(@definition.live_component?)
  end

  def test_name
    assert_instance_of(String, @definition.name)
    assert_equal('Heather', @definition.name)
    assert_equal(@definition.name = 'Test Name', 'Test Name')
    assert_equal('Test Name', @definition.name)
  end

  def test_path
    assert_instance_of(String, @definition.path)
  end


  def test_save_as
    Dir.mktmpdir { |dir|
      path = "#{dir}/test.skp"
      @definition.save_as(path)
      assert(File.exist?(path))
    }
    assert_raises(TypeError) {
      @definition.save_as(Object.new)
    }
  end

  def test_save_as_version
    Dir.mktmpdir { |dir|
      path = "#{dir}/test.skp"
      @definition.save_as(path, 8)
      assert(File.exist?(path))
      assert_raises(TypeError) {
        @definition.save_as(path, 'test')
      }
    }
    assert_raises(TypeError) {
      @definition.save_as(Object.new, 8)
    }
  end

  def test_unsupported
    assert_unsupported(@definition, UNSUPPORTED)
  end

  def test_comparable
    assert_kind_of(Comparable, @definition)
  end

  def test_insertion_point
    assert_equal(Geom::Point3d.new(0, 0, 0), @definition.insertion_point)
  end

  def test_load_time
    assert_instance_of(Time, @definition.load_time)
  end

  def test_save_copy
    Dir.mktmpdir { |dir|
      path = "#{dir}/copy.skp"
      assert(@definition.save_copy(path))
      assert(File.exist?(path))
    }
  end
end
