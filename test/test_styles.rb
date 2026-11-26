require_relative 'helper'

class TestStyles < Minitest::Test
  UNSUPPORTED = %i[purge_unused update_selected_style].freeze # no C API

  def setup
    @model = open_model
    @styles = @model.styles
  end

  def test_unsupported
    assert_unsupported(@styles, UNSUPPORTED)
  end

  def test_class
    assert_instance_of(Sketchup::Styles, @styles)
    assert_kind_of(Enumerable, @styles)
    assert_equal(@model, @styles.parent)
  end

  def test_each_and_counts
    seen = []
    assert_nil(@styles.each { |s| seen << s })
    refute_empty(seen)
    seen.each { |s| assert_instance_of(Sketchup::Style, s) }
    assert_equal(seen.size, @styles.count)
    assert_equal(seen.size, @styles.length)
    assert_equal(seen.size, @styles.size)
  end

  def test_index
    style = @styles[0]
    assert_instance_of(Sketchup::Style, style)
    assert_equal(style, @styles[style.name])
    assert_nil(@styles['No such style'])
  end

  def test_active_and_selected
    assert_instance_of(Sketchup::Style, @styles.active_style)
    assert_instance_of(Sketchup::Style, @styles.selected_style)
    assert_includes([true, false], @styles.active_style_changed)
    @styles.selected_style = @styles[0]
    assert_equal(@styles[0], @styles.selected_style)
  end

  def test_style
    style = @styles[0]
    assert_instance_of(String, style.path)
    assert_equal('Renamed', style.name = 'Renamed')
    assert_equal('Renamed', style.name)
    assert_equal('About', style.description = 'About')
    assert_equal('About', style.description)
  end

  def test_add_and_remove_style
    assert_equal(false, @styles.add_style('/does/not/exist.style', false))
    count = @styles.count
    style = @styles[0]
    assert_nil(@styles.remove_style(style)) if count > 1
    assert_raises(TypeError) { @styles.remove_style('x') }
  end
end
