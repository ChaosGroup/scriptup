require 'minitest/autorun'
require 'sketchup'

class TestDrawingElement < Minitest::Test

  def setup
    Sketchup.open_file("#{__dir__}/../test-resources/Untitled.skp")
    @drawing_element = Sketchup.active_model.definitions.find { |d| d.name == 'Heather' }
  end

  def test_object
    assert_kind_of(Sketchup::DrawingElement, @drawing_element)
  end

  def test_bounds
    assert_kind_of(Geom::BoundingBox, @drawing_element.bounds)
  end

  def test_cast_shadows
    assert(@drawing_element.entities.grep(Sketchup::Face).first.casts_shadows = true)
    assert(@drawing_element.entities.grep(Sketchup::Face).first.casts_shadows?)
    refute(@drawing_element.entities.grep(Sketchup::Face).first.casts_shadows = false)
    refute(@drawing_element.entities.grep(Sketchup::Face).first.casts_shadows?)
  end

  def test_hidden
    refute(@drawing_element.hidden?)
    assert(@drawing_element.hidden = true)
  end

  def test_layer
    assert_kind_of(Sketchup::Layer, @drawing_element.layer)
    assert_raises(ArgumentError) {
      @drawing_element.layer = Object.new
    }
  end

  def test_material
    assert_kind_of(Sketchup::Material, @drawing_element.material)
    assert_raises(TypeError) {
      @drawing_element.material = Object.new
    }
  end

  def test_receives_shadows
    assert(@drawing_element.receives_shadows?)
    assert(@drawing_element.receives_shadows = true)
  end

  def test_visible
    assert(@drawing_element.visible?)
    refute(@drawing_element.visible = false)
  end

end
