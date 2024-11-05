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

  def test_each
    counter = 0
    assert_equal(@materials.each { |material|
      assert_instance_of(Sketchup::Material, material)
      counter += 1
    }, @materials)
    assert_equal(19, counter)
  end

end
