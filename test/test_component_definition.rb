require 'minitest/autorun'
require 'sketchup'

class TestComponentDefinition < Minitest::Test

  def setup
    Sketchup.open_file("#{ENV['TEST_RESOURCES']}/Untitled.skp")
    @definition = Sketchup.active_model.definitions.find { |d| d.name == 'Heather' }
  end

  def test_object
    assert_kind_of(Sketchup::DrawingElement, @definition)
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

  def test_add_observer
    assert(@definition.add_observer(nil))
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

  def test_remove_observer
    assert(@definition.remove_observer(42) == false)
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

end
