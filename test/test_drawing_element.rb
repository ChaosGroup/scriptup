require_relative 'helper'

class TestDrawingElement < Minitest::Test
  def setup
    @model = open_model
    @definition = @model.definitions['Heather']
    @face = add_square_face(@model.entities)
  end

  def test_class
    assert_kind_of(Sketchup::Entity, @face)
    assert_kind_of(Sketchup::Drawingelement, @face)
    assert_kind_of(Sketchup::Drawingelement, @definition)
  end

  def test_bounds
    bounds = @face.bounds
    assert_instance_of(Geom::BoundingBox, bounds)
    assert_equal(10, bounds.width)
    assert_equal(10, bounds.height)
  end

  def test_casts_shadows
    assert_equal(true, @face.casts_shadows = true)
    assert(@face.casts_shadows?)
    assert_equal(false, @face.casts_shadows = false)
    refute(@face.casts_shadows?)
  end

  def test_receives_shadows
    assert(@face.receives_shadows?)
    @face.receives_shadows = false
    refute(@face.receives_shadows?)
  end

  def test_hidden_and_visible
    refute(@face.hidden?)
    assert(@face.visible?)
    assert_equal(true, @face.hidden = true)
    assert(@face.hidden?)
    refute(@face.visible?)
    assert_equal(true, @face.visible = true)
    refute(@face.hidden?)
    @face.visible = false
    assert(@face.hidden?)
  end

  def test_layer
    assert_kind_of(Sketchup::Layer, @face.layer)
    layer = @face.layer
    assert_equal(layer, @face.layer = layer)
    assert_nil(@face.layer = nil)
    assert_raises(ArgumentError) { @face.layer = Object.new }
    assert_raises(ArgumentError) { @face.layer = 'No such tag' }
  end

  def test_material
    assert_nil(@face.material)
    material = @model.materials['Heather_Shirt']
    assert_equal(material, @face.material = material)
    assert_equal(material, @face.material)
    assert_equal('Heather_Jeans', @face.material = 'Heather_Jeans')
    assert_equal('Heather_Jeans', @face.material.name)
    color = Sketchup::Color.new(255, 0, 0)
    count = @model.materials.count
    @face.material = color
    assert_equal(color, @face.material.color)
    assert_equal(count + 1, @model.materials.count)
    assert_nil(@face.material = nil)
    assert_nil(@face.material)
    assert_raises(TypeError) { @face.material = Object.new }
    assert_raises(ArgumentError) { @face.material = 'No such material' }
  end

  def test_erase!
    count = @model.entities.count
    edge = @model.entities.add_line([0, 0, 50], [1, 0, 50])
    assert_nil(edge.erase!)
    assert_equal(count, @model.entities.count)
  end
end
