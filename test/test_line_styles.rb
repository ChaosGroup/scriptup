require_relative 'helper'

class TestLineStyles < Minitest::Test
  def setup
    @model = open_model
    @styles = @model.line_styles
  end

  def test_class
    assert_instance_of(Sketchup::LineStyles, @styles)
    assert_kind_of(Enumerable, @styles)
  end

  def test_names_and_size
    names = @styles.names
    refute_empty(names)
    names.each { |n| assert_instance_of(String, n) }
    assert_equal(names.size, @styles.size)
  end

  def test_each_and_to_a
    seen = []
    assert_nil(@styles.each { |s| seen << s })
    assert_equal(@styles.size, seen.size)
    seen.each { |s| assert_instance_of(Sketchup::LineStyle, s) }
    assert_equal(seen, @styles.to_a)
  end

  def test_index
    name = @styles.names.first
    style = @styles[name]
    assert_instance_of(Sketchup::LineStyle, style)
    assert_equal(name, style.name)
    assert_equal(style, @styles.at(name))
    assert_equal(style, @styles[0])
    assert_nil(@styles['No such style'])
    assert_nil(@styles[999])
  end

  def test_layer_line_style
    layer = @model.layers.add('Styled')
    style = @styles[@styles.names.last]
    assert_equal(style, layer.line_style = style)
    assert_equal(style, layer.line_style)
    assert_equal(style.name, layer.line_style.name)
  end
end
