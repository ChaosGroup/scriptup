require 'minitest/autorun'
require 'sketchup'

class TestTexture < Minitest::Test

  def setup
    Sketchup.open_file("#{ENV['TEST_RESOURCES']}/Untitled.skp")
    material = Sketchup.active_model.materials['Heather_Shirt']
    material.texture = "#{__dir__}/texture_map.png"
    @texture = material.texture
  end

  def test_average_color
    assert_instance_of(Sketchup::Color, @texture.average_color)
  end

  def test_filename
  end

  def test_height
  end

  def test_image_height
  end

  def test_image_rep
  end
  
  def test_image_width
  end

  def test_size
  end

  def test_valid?
  end

  def test_width
  end

  def test_write
  end

end
