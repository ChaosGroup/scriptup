require_relative 'helper'

class TestImageRep < Minitest::Test
  def setup
    open_model
    @path = File.join(TEST_RESOURCES, 'texture_map.png')
    @image = Sketchup::ImageRep.new(@path)
  end

  def test_initialize
    empty = Sketchup::ImageRep.new
    assert_equal(0, empty.width)
    assert_nil(empty.data)
    assert_nil(empty.colors)
    assert_raises(ArgumentError) { Sketchup::ImageRep.new('/does/not/exist.png') }
  end

  def test_dimensions
    assert_equal(1024, @image.width)
    assert_equal(1024, @image.height)
    assert_instance_of(Integer, @image.row_padding)
  end

  def test_data
    assert_includes([24, 32], @image.bits_per_pixel)
    assert_equal(@image.size, @image.data.bytesize)
    assert_operator(@image.size, :>=, 1024 * 1024 * 3)
  end

  def test_colors
    colors = @image.colors
    assert_equal(1024 * 1024, colors.size)
    assert_instance_of(Sketchup::Color, colors.first)
  end

  def test_color_at_uv
    assert_instance_of(Sketchup::Color, @image.color_at_uv(0.5, 0.5))
    assert_instance_of(Sketchup::Color, @image.color_at_uv(0.5, 0.5, true))
  end

  def test_set_data
    # Pixel bytes are BGRA on Windows and RGBA on macOS.
    red = Gem.win_platform? ? [0, 0, 255, 255] : [255, 0, 0, 255]
    pixels = (red * 4).pack('C*')
    image = Sketchup::ImageRep.new
    assert_same(image, image.set_data(2, 2, 32, 0, pixels))
    assert_equal(2, image.width)
    assert_equal(32, image.bits_per_pixel)
    assert_equal(Sketchup::Color.new(255, 0, 0), image.colors.first.tap { |c| c.alpha = 255 })
    assert_raises(ArgumentError) { image.set_data(4, 4, 32, 0, pixels) }
  end

  def test_load_and_save
    image = Sketchup::ImageRep.new
    assert_same(image, image.load_file(@path))
    assert_equal(1024, image.width)
    Dir.mktmpdir do |dir|
      out = File.join(dir, 'copy.png')
      assert_same(image, image.save_file(out))
      assert(File.exist?(out))
    end
  end

  def test_from_texture
    material = Sketchup.active_model.materials['Heather_Shirt']
    material.texture = @path
    assert_equal(1024, material.texture.image_rep.width)
  end
end
