require 'minitest/autorun'
require 'sketchup'

class TestColor < Minitest::Test

  def setup
    Sketchup.open_file("#{ENV['TEST_RESOURCES']}/Untitled.skp")
    @color = Sketchup.active_model.materials['Heather_Shirt'].color
  end

  def test_names
    assert_equal(Sketchup::Color.names, ['AliceBlue', 'AntiqueWhite', 'Aqua', 'Aquamarine', 'Azure', 'Beige', 'Bisque', 'Black', 'BlanchedAlmond', 'Blue', 'BlueViolet', 'Brown', 'BurlyWood', 'CadetBlue', 'Chartreuse', 'Chocolate', 'Coral', 'CornflowerBlue', 'Cornsilk', 'Crimson', 'Cyan', 'DarkBlue', 'DarkCyan', 'DarkGoldenrod', 'DarkGray', 'DarkGreen', 'DarkKhaki', 'DarkMagenta', 'DarkOliveGreen', 'DarkOrange', 'DarkOrchid', 'DarkRed', 'DarkSalmon', 'DarkSeaGreen', 'DarkSlateBlue', 'DarkSlateGray', 'DarkTurquoise', 'DarkViolet', 'DeepPink', 'DeepSkyBlue', 'DimGray', 'DodgerBlue', 'FireBrick', 'FloralWhite', 'ForestGreen', 'Fuchsia', 'Gainsboro', 'GhostWhite', 'Gold', 'Goldenrod', 'Gray', 'Green', 'GreenYellow', 'Honeydew', 'HotPink', 'IndianRed', 'Indigo', 'Ivory', 'Khaki', 'Lavender', 'LavenderBlush', 'LawnGreen', 'LemonChiffon', 'LightBlue', 'LightCoral', 'LightCyan', 'LightGoldenrodYellow', 'LightGreen', 'LightGrey', 'LightPink', 'LightSalmon', 'LightSeaGreen', 'LightSkyBlue', 'LightSlateGray', 'LightSteelBlue', 'LightYellow', 'Lime', 'LimeGreen', 'Linen', 'Magenta', 'Maroon', 'MediumAquamarine', 'MediumBlue', 'MediumOrchid', 'MediumPurple', 'MediumSeaGreen', 'MediumSlateBlue', 'MediumSpringGreen', 'MediumTurquoise', 'MediumVioletRed', 'MidnightBlue', 'MintCream', 'MistyRose', 'Moccasin', 'NavajoWhite', 'Navy', 'OldLace', 'Olive', 'OliveDrab', 'Orange', 'OrangeRed', 'Orchid', 'PaleGoldenrod', 'PaleGreen', 'PaleTurquoise', 'PaleVioletRed', 'PapayaWhip', 'PeachPuff', 'Peru', 'Pink', 'Plum', 'PowderBlue', 'Purple', 'Red', 'RosyBrown', 'RoyalBlue', 'SaddleBrown', 'Salmon', 'SandyBrown', 'SeaGreen', 'Seashell', 'Sienna', 'Silver', 'SkyBlue', 'SlateBlue', 'SlateGray', 'Snow', 'SpringGreen', 'SteelBlue', 'Tan', 'Teal', 'Thistle', 'Tomato', 'Turquoise', 'Violet', 'Wheat', 'White', 'WhiteSmoke', 'Yellow', 'YellowGreen'])
  end

  def test_initialize
    red1 = Sketchup::Color.new('Red')
    red2 = Sketchup::Color.new(0x0000FF)
    red3 = Sketchup::Color.new(255, 0, 0)
    red4 = Sketchup::Color.new(255, 0, 0, 128)
    assert(red1 == red2)
    assert(red1 == red3)
    assert(red1 != red4)
    assert(red4.alpha, 128)
  end

  def test_equals
    color = Sketchup::Color.new(48, 79, 73)
    assert(@color == color)
  end

  def test_alpha
    assert_equal(@color.alpha, 255)
    assert_equal(@color.alpha = 200, 200)
  end

  def test_blend
    color = Sketchup::Color.new(10, 10, 10)
    assert_instance_of(Sketchup::Color, @color.blend(color, 0.5))
    color1 = @color.blend(color, 0.5)
    assert_equal(color1.to_a, [29, 44, 41, 255])
  end

  def test_blue
    assert_equal(@color.blue, 73)
    assert_equal(@color.blue = 55, 55)
  end

  def test_green
    assert_equal(@color.green, 79)
    assert_equal(@color.green = 55, 55)
  end

  def test_red
    assert_equal(@color.red, 48)
    assert_equal(@color.red = 55, 55)
  end

  def test_to_a
    assert_equal(@color.to_a, [48, 79, 73, 255])
  end

  def test_to_i
    assert_equal(@color.to_i, 4_804_400)
  end

  def test_to_s
    assert_equal(@color.to_s, 'Color(48, 79, 73, 255)')
  end

end
