require 'minitest/autorun'
require 'sketchup'

class TestDefinitionList < Minitest::Test

  def setup
    Sketchup.open_file("#{ENV['TEST_RESOURCES']}/Untitled.skp")
    @definitions = Sketchup.active_model.definitions
  end

  def test_object
    assert_kind_of(Enumerable, @definitions)
  end

  def test_get
    assert_raises(IndexError) {
      Sketchup.active_model.definitions[3]
    }
    assert_raises(TypeError) {
      Sketchup.active_model.definitions[Object.new]
    }
    assert_equal('Heather', Sketchup.active_model.definitions['Heather'].name)
    assert_equal('Heather', Sketchup.active_model.definitions[0].name)
    assert_equal('Heather', Sketchup.active_model.definitions['0BRlipAQn5NfsR9CLTgK7v'].name)
  end

  def test_add
    refute(@definitions.map(&:name).include?('TestDefinition'))
    @definitions.add('TestDefinition')
    assert(@definitions.map(&:name).include?('TestDefinition'))
    assert_raises(TypeError) {
      @definitions.add(Object.new)
    }
    assert_raises(TypeError) {
      @definitions.add(1)
    }
  end

  def test_add_observer
    assert(@definitions.add_observer(nil))
  end

  def test_count
    assert_equal(2, @definitions.count)
  end

  def test_each
    assert_raises(LocalJumpError) { @definitions.each }
    counter = 0
    assert_equal(@definitions.each { |definition|
      assert_instance_of(Sketchup::ComponentDefinition, definition)
      counter += 1
    }, @definitions)
    assert_equal(2, counter)
  end

  def test_length
    assert_equal(2, @definitions.length)
  end

  def test_load
    assert_raises(StandardError) {
      @definitions.load("#{ENV['TEST_RESOURCES']}/Untitled.skp")
    }
    assert_instance_of(Sketchup::ComponentDefinition, @definitions.load("#{ENV['TEST_RESOURCES']}/Heather.skp"))
  end

  def test_remove
    assert(2, @definitions.count)
    assert_raises(TypeError) {
      @definitions.remove(Object.new)
    }
    @definitions.remove(@definitions[0])
    assert(1, @definitions.count)
  end

  def test_remove_observer
    assert(@definitions.remove_observer(42) == false)
  end

  def test_size
    assert(2, @definitions.size)
  end

end
