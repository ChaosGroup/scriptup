require_relative 'helper'

class TestImage < Minitest::Test
  UNSUPPORTED = %i[explode glued_to glued_to=].freeze # no C API for images

  def setup
    @model = open_model
    @path = File.join(TEST_RESOURCES, 'texture_map.png')
    @image = @model.entities.add_image(@path, [1, 2, 3], 100)
  end

  def test_unsupported
    assert_unsupported(@image, UNSUPPORTED)
  end

  def test_path
    assert_equal('texture_map.png', File.basename(@image.path))
  end

  def test_size
    assert_instance_of(Length, @image.width)
    assert_in_delta(100.0, @image.width.to_f)
    assert_in_delta(100.0, @image.height.to_f) # square image
  end

  def test_resize
    assert_equal(50, @image.width = 50)
    assert_in_delta(50.0, @image.width.to_f)
    assert_in_delta(50.0, @image.height.to_f, 1e-6)
    @image.height = 20
    assert_in_delta(20.0, @image.width.to_f, 1e-6)
    assert_equal([30, 60], (@image.size = 30, 60))
    assert_in_delta(30.0, @image.width.to_f)
    assert_in_delta(60.0, @image.height.to_f)
    assert_raises(ArgumentError) { @image.width = 0 }
  end

  def test_pixels
    assert_equal(1024, @image.pixelwidth)
    assert_equal(1024, @image.pixelheight)
  end

  def test_image_rep
    rep = @image.image_rep
    assert_instance_of(Sketchup::ImageRep, rep)
    assert_equal(1024, rep.width)
  end

  def test_origin
    assert_equal(Geom::Point3d.new(1, 2, 3), @image.origin)
    assert_equal([5, 5, 5], @image.origin = [5, 5, 5])
    assert_equal(Geom::Point3d.new(5, 5, 5), @image.origin)
  end

  def test_normal_and_zrotation
    assert(@image.normal.samedirection?([0, 0, 1]))
    assert_in_delta(0.0, @image.zrotation)
    @image.transform!(Geom::Transformation.rotation(@image.origin, [0, 0, 1], 90.degrees))
    assert_in_delta(Math::PI / 2, @image.zrotation)
  end

  def test_transformation
    assert_instance_of(Geom::Transformation, @image.transformation)
    t = Geom::Transformation.translation([9, 9, 9])
    assert_equal(t, @image.transformation = t)
    assert_equal(Geom::Point3d.new(9, 9, 9), @image.origin)
    assert_same(@image, @image.transform!(Geom::Transformation.translation([1, 0, 0])))
    assert_equal(Geom::Point3d.new(10, 9, 9), @image.origin)
  end
end
