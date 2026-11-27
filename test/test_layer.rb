require_relative 'helper'

class TestLayer < Minitest::Test
  def setup
    @model = open_model
    @layer = @model.layers.add('Walls')
  end

  def test_constants
    assert_equal(0, LAYER_VISIBLE_BY_DEFAULT)
    assert_equal(1, LAYER_HIDDEN_BY_DEFAULT)
    assert_equal(0, LAYER_USES_DEFAULT_VISIBILITY_ON_NEW_PAGES)
    assert_equal(0x10, LAYER_IS_VISIBLE_ON_NEW_PAGES)
    assert_equal(0x20, LAYER_IS_HIDDEN_ON_NEW_PAGES)
  end

  def test_name
    assert_equal('Walls', @layer.name)
    assert_equal('Floors', @layer.name = 'Floors')
    assert_equal('Floors', @layer.name)
    assert_equal('Floors', @layer.display_name)
    assert_raises(TypeError) { @layer.name = 1 }
  end

  def test_comparable
    assert_kind_of(Comparable, @layer)
    other = @model.layers.add('Zones')
    assert_equal(-1, @layer <=> other)
    assert_equal(0, @layer <=> @layer)
    assert_nil(@layer <=> 1)
    assert(@layer == @model.layers['Walls'])
  end

  def test_visible
    assert(@layer.visible?)
    assert_equal(false, @layer.visible = false)
    refute(@layer.visible?)
  end

  def test_color
    assert_instance_of(Sketchup::Color, @layer.color)
    color = Sketchup::Color.new(10, 20, 30)
    assert_equal(color, @layer.color = color)
    assert_equal(color, @layer.color)
    @layer.color = [1, 2, 3]
    assert_equal(Sketchup::Color.new(1, 2, 3), @layer.color)
  end

  def test_line_style
    assert_nil(@layer.line_style)
    assert_nil(@layer.line_style = nil)
    assert_raises(TypeError) { @layer.line_style = 'Dashed' }
  end

  def test_folder
    assert_nil(@layer.folder)
    folder = @model.layers.add_folder('Building')
    assert_equal(folder, @layer.folder = folder)
    assert_equal(folder, @layer.folder)
    assert_nil(@layer.folder = nil)
    assert_nil(@layer.folder)
  end

  def test_page_behavior
    assert_equal(LAYER_VISIBLE_BY_DEFAULT, @layer.page_behavior)
    behavior = LAYER_HIDDEN_BY_DEFAULT | LAYER_IS_VISIBLE_ON_NEW_PAGES
    assert_equal(behavior, @layer.page_behavior = behavior)
    assert_equal(behavior, @layer.page_behavior)
  end
end
