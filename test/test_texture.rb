require 'minitest/autorun'
require 'sketchup'

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

  def test_image_rep
    assert_instance_of(Sketchup::ImageRep, @texture.image_rep)
  end
  
  def test_image_width
    assert_equal(1024, @texture.image_width)
  end

  def test_size
  end

  def test_valid?
  end

  def test_write
  end

end
