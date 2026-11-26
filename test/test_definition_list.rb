require_relative 'helper'

class TestDefinitionList < Minitest::Test
  UNSUPPORTED = %i[load_from_url add_observer remove_observer].freeze # no C API

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


  def test_size
    assert(2, @definitions.size)
  end

  def test_unsupported
    assert_unsupported(@definitions, UNSUPPORTED)
  end

  def test_at
    assert_equal(@definitions[0], @definitions.at(0))
    assert_equal('Heather', @definitions.at('Heather').name)
  end

  def test_load_allow_newer
    definition = @definitions.load("#{ENV['TEST_RESOURCES']}/Heather.skp", allow_newer: true)
    assert_instance_of(Sketchup::ComponentDefinition, definition)
  end

  def test_import
    assert_instance_of(Sketchup::ComponentDefinition, @definitions.import("#{ENV['TEST_RESOURCES']}/Heather.skp"))
    assert_raises(NotImplementedError) { @definitions.import('model.dae') }
  end

  def test_purge_unused
    @definitions.add('Unused')
    assert(@definitions.map(&:name).include?('Unused'))
    assert_same(@definitions, @definitions.purge_unused)
    refute(@definitions.map(&:name).include?('Unused'))
    assert(@definitions.map(&:name).include?('Heather'))
  end

  def test_unique_name
    assert_equal('Fresh', @definitions.unique_name('Fresh'))
    assert_equal('Heather#1', @definitions.unique_name('Heather'))
    @definitions.add('Heather#1')
    assert_equal('Heather#2', @definitions.unique_name('Heather'))
    assert_raises(TypeError) { @definitions.unique_name(1) }
  end
end
