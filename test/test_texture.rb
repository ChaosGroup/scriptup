require_relative 'helper'

class TestTexture < Minitest::Test

  def setup
    Sketchup.open_file("#{ENV['TEST_RESOURCES']}/Untitled.skp")
    @material = Sketchup.active_model.materials['Heather_Shirt']
    @material.texture = "#{ENV['TEST_RESOURCES']}/texture_map.png"
    @texture = @material.texture
  end

  def test_average_color
    assert_instance_of(Sketchup::Color, @texture.average_color)
  end

  def test_filename
    assert_instance_of(String, @texture.filename)
  end

  def test_image_height
    assert_equal(1024, @texture.image_height)
  end

  def test_height
    assert_equal(1.0, @texture.height)
  end

  def test_image_width
    assert_equal(1024, @texture.image_width)
  end

  def test_width
    assert_equal(1.0, @texture.width)
  end

  def test_image_rep
    assert_instance_of(Sketchup::ImageRep, @texture.image_rep)
  end
  
  def test_valid?
    assert(@texture.valid?)
  end

  def test_write
    basename = @texture.filename
    Dir.mktmpdir { |dir|
        path = File.join("#{dir}", basename)
        refute(File.exist?(path))
        @texture.write(path, true)
        assert(File.exist?(path))
    }
    Dir.mktmpdir { |dir|
        path = File.join("#{dir}", basename)
        refute(File.exist?(path))
        @texture.write(path, false)
        assert(File.exist?(path))
    }
  end

  def test_image_rep_colorized
    assert_instance_of(Sketchup::ImageRep, @texture.image_rep(true))
  end

  def test_size
    assert_equal(10, @texture.size = 10)
    assert_in_delta(10.0, @texture.width)
    assert_in_delta(10.0, @texture.height, 1e-6) # square image keeps its ratio
    assert_equal([10, 100], @texture.size = [10, 100])
    assert_in_delta(10.0, @texture.width)
    assert_in_delta(100.0, @texture.height)
    assert_raises(ArgumentError) { @texture.size = 0 }
    assert_raises(ArgumentError) { @texture.size = [1, 2, 3] }
  end
end
