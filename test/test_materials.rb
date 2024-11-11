require 'minitest/autorun'
require 'sketchup'

class TestMaterialsList < Minitest::Test

  def setup
    Sketchup.open_file("#{ENV['TEST_RESOURCES']}/Untitled.skp")
    @materials = Sketchup.active_model.materials
  end

  def test_object
    assert_kind_of(Enumerable, @materials)
  end

  def test_get
    assert_raises(IndexError) {
      Sketchup.active_model.materials[19]
    }
    assert_raises(TypeError) {
      Sketchup.active_model.materials[Object.new]
    }
    assert_equal(Sketchup.active_model.materials['Heather_Shirt'].name, 'Heather_Shirt')
    assert_equal(Sketchup.active_model.materials[1].name, 'Heather_Shirt')
  end

  def test_add_observer
    assert(@materials.add_observer(nil))
  end
  
  def test_count
    assert_equal(19, @materials.count)
  end

  def test_current
    assert_nil(@materials.current)
    @materials.current = @materials[0]
    assert_instance_of(Sketchup::Material, @materials.current)
  end

  def test_each
    counter = 0
    assert_equal(@materials.each { |material|
      assert_instance_of(Sketchup::Material, material)
      counter += 1
    }, @materials)
    assert_equal(19, counter)
  end

  def test_length
    assert_equal(19, @materials.length)
  end

  def test_load
    assert_nil(@materials['test_material'])
    @materials.load("#{ENV['TEST_RESOURCES']}/test_material.skm")
    assert_instance_of(Sketchup::Material, @materials['test_material'])
    assert_raises(RuntimeError) {
      @materials.load('error')
    }
    assert_raises(TypeError) {
      @materials.load(Object.new)
    }
  end
  
  def test_remove
    loaded_material = @materials.load("#{ENV['TEST_RESOURCES']}/test_material.skm")
    assert_instance_of(Sketchup::Material, @materials['test_material'])
    assert(@materials.remove(loaded_material))
    assert_nil(@materials['test_material'])
    assert_raises(ArgumentError) {
      @materials.remove('test_material')
    }
    assert_raises(TypeError) {
      @materials.remove(Object.new)
    }
  end

  def test_remove_observer
    assert(@materials.remove_observer(42) == false)
  end

  def test_size
    assert_equal(19, @materials.size)
  end

  def test_unique_name
    assert_equal('test name', @materials.unique_name('test_name'))
  end

end
