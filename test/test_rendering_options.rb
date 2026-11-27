require_relative 'helper'

class TestRenderingOptions < Minitest::Test
  UNSUPPORTED = %i[add_observer remove_observer].freeze # no C API

  def setup
    @model = open_model
    @options = @model.rendering_options
  end

  def test_unsupported
    assert_unsupported(@options, UNSUPPORTED)
  end

  def test_class
    assert_instance_of(Sketchup::RenderingOptions, @options)
    assert_kind_of(Enumerable, @options)
  end

  def test_keys
    keys = @options.keys
    assert_includes(keys, 'DisplayFog')
    assert_equal(keys, Sketchup::RenderingOptions.keys)
    assert_equal(keys.size, @options.count)
    assert_equal(keys.size, @options.length)
    assert_equal(keys.size, @options.size)
  end

  def test_each
    pairs = []
    assert_nil(@options.each { |k, v| pairs << [k, v] })
    assert_equal(@options.keys, pairs.map(&:first))
    assert_equal(pairs, @options.to_a)
    keys = []
    @options.each_key { |k| keys << k }
    assert_equal(@options.keys, keys)
    class_keys = []
    Sketchup::RenderingOptions.each_key { |k| class_keys << k }
    assert_equal(@options.keys, class_keys)
  end

  def test_values
    assert_includes([true, false], @options['DisplayFog'])
    assert_instance_of(Sketchup::Color, @options['BackgroundColor'])
    assert_nil(@options['NoSuchKey'])
  end

  def test_set
    assert_equal(true, @options['DisplayFog'] = true)
    assert_equal(true, @options['DisplayFog'])
    color = Sketchup::Color.new(10, 20, 30)
    @options['BackgroundColor'] = color
    assert_equal(color, @options['BackgroundColor'])
    assert_raises(ArgumentError) { @options['NoSuchKey'] = 1 }
  end
end
