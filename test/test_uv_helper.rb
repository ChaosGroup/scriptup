require_relative 'helper'

class TestUVHelper < Minitest::Test
  def setup
    @model = open_model
    material = @model.materials['Heather_Shirt']
    material.texture = File.join(TEST_RESOURCES, 'texture_map.png')
    material.texture.size = 10
    @face = add_square_face(@model.entities)
    @face.material = material
    @face.back_material = material
  end

  def test_get_UVHelper
    helper = @face.get_UVHelper
    assert_instance_of(Sketchup::UVHelper, helper)
    assert_instance_of(Sketchup::UVHelper, @face.get_UVHelper(true, false))
    assert_instance_of(Sketchup::UVHelper, @face.get_UVHelper(true, true, Sketchup.create_texture_writer))
    assert_raises(TypeError) { @face.get_UVHelper(true, true, 'x') }
    assert_raises(NoMethodError) { Sketchup::UVHelper.new }
  end

  def test_front_uvq
    uvq = @face.get_UVHelper.get_front_UVQ(Geom::Point3d.new(5, 5, 0))
    assert_instance_of(Geom::Point3d, uvq)
    assert_in_delta(1.0, uvq.z.to_f)
    origin = @face.get_UVHelper.get_front_UVQ(Geom::Point3d.new(0, 0, 0))
    assert_in_delta(0.5, (uvq.x - origin.x).abs, 1e-6, 'a 10" texture repeats every 10"')
  end

  def test_back_uvq
    assert_instance_of(Geom::Point3d, @face.get_UVHelper.get_back_UVQ([5, 5, 0]))
  end
end
