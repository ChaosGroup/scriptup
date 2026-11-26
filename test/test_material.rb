require_relative 'helper'

class TestMaterial < Minitest::Test
  UNSUPPORTED = %i[write_thumbnail].freeze # no C API

  def setup
    Sketchup.open_file("#{ENV['TEST_RESOURCES']}/Untitled.skp")
    @material = Sketchup.active_model.materials['Heather_Shirt']
  end

  def test_spaceship_operator
    assert_equal(0, @material <=> @material)
    jeans = Sketchup.active_model.materials['Heather_Jeans']
    assert_equal(1, @material <=> jeans)
    assert_equal(-1, jeans <=> @material)
    assert_raises {
      refute(@material <=> 42)
    }
  end

  def test_equals
    assert(@material == @material)
    jeans = Sketchup.active_model.materials['Heather_Jeans']
    jeans_by_index = Sketchup.active_model.materials[9]
    refute(@material == jeans)
    refute(@material == jeans_by_index)
    refute(@material == Object.new)
  end

  def test_alpha
    assert_equal(@material.alpha, 1.0)
    assert_equal(@material.alpha = 0.5, 0.5)
    assert_equal(@material.alpha, 0.5)
    assert_equal(@material.alpha = -3, -3)
    assert_equal(@material.alpha, 0)
    assert_equal(@material.alpha = 2, 2)
    assert_equal(@material.alpha, 1.0)
    assert_raises(TypeError) {
      @material.alpha = Object.new
    }
  end

  def test_color
    assert_instance_of(Sketchup::Color, @material.color)
    assert_equal(@material.color = [1011, 50, 70], [1011, 50, 70])
    assert_raises(ArgumentError) {
      @material.color = [255, 50]
    }
    assert_raises(TypeError) {
      @material.color = [255, 50, Object.new]
    }
    color = Sketchup::Color.new(42, 43, 44)
    assert_equal(@material.color = color, color)
    assert_equal(@material.color, color)
    assert_equal(@material.color = [1011, 50, -10], [1011, 50, -10])
    color2 = Sketchup::Color.new(255, 50, 0)
    assert_equal(@material.color, color2)
  end

  def test_colorize_deltas
    assert_equal(@material.colorize_deltas, [0.0, 0.0, 0.0])
  end

  def test_colorize_type
    assert_equal(@material.colorize_type, 0)
    assert_equal(@material.colorize_type = 1, 1)
    assert_equal(@material.colorize_type, 1)
    assert_raises(TypeError) {
      @material.colorize_type = Object.new
    }
  end

  def test_display_name
    assert_equal(@material.display_name, 'Heather_Shirt')
  end

  def test_materialType
    assert_equal(@material.materialType, 0)
  end

  def test_name
    assert_equal(@material.name, 'Heather_Shirt')
    assert_equal(@material.name = 'Foo', 'Foo')
    assert_equal(@material.name, 'Foo')
    assert_raises(TypeError) {
      @material.name = Object.new
    }
  end

  def test_owner_type
    assert_equal(@material.owner_type, 0)
  end

  def test_save_as
    Dir.mktmpdir { |dir|
      path = "#{dir}/test.skm"
      @material.save_as(path)
      assert(File.exist?(path))
    }
    assert_raises(TypeError) {
      @material.save_as(Object.new)
    }
  end

  def test_texture
    refute(@material.texture)
    @material.texture = "#{__dir__}/texture_map.png"
    assert_instance_of(String, @material.texture = "#{ENV['TEST_RESOURCES']}/texture_map.png")
    assert_instance_of(Sketchup::Texture, @material.texture)
    refute(@material.texture = nil)
    refute(@material.texture)
    assert_raises(ArgumentError) {
      @material.texture = Object.new
    }
  end

  def test_use_alpha?
    refute(@material.use_alpha?)
  end

  def test_unsupported
    assert_unsupported(@material, UNSUPPORTED)
  end

  def test_constants
    assert_equal(0, Sketchup::Material::COLORIZE_SHIFT)
    assert_equal(1, Sketchup::Material::COLORIZE_TINT)
    assert_equal([0, 1, 2], [Sketchup::Material::MATERIAL_SOLID, Sketchup::Material::MATERIAL_TEXTURED, Sketchup::Material::MATERIAL_COLORIZED_TEXTURED])
    assert_equal([0, 1, 2], [Sketchup::Material::OWNER_MANAGER, Sketchup::Material::OWNER_IMAGE, Sketchup::Material::OWNER_LAYER])
    assert_equal([0, 1], [Sketchup::Material::NORMAL_STYLE_OPENGL, Sketchup::Material::NORMAL_STYLE_DIRECTX])
    assert_equal([0, 1], [Sketchup::Material::WORKFLOW_CLASSIC, Sketchup::Material::WORKFLOW_PBR_METALLIC_ROUGHNESS])
  end

  def test_workflow
    assert_equal(Sketchup::Material::WORKFLOW_CLASSIC, @material.workflow)
    @material.metalness_enabled = true
    assert_equal(Sketchup::Material::WORKFLOW_PBR_METALLIC_ROUGHNESS, @material.workflow)
  end

  def test_metalness
    refute(@material.metalness_enabled?)
    assert_equal(true, @material.metalness_enabled = true)
    assert(@material.metalness_enabled?)
    assert_equal(0.5, @material.metallic_factor = 0.5)
    assert_in_delta(0.5, @material.metallic_factor)
  end

  def test_roughness
    @material.roughness_enabled = true
    assert(@material.roughness_enabled?)
    @material.roughness_factor = 0.25
    assert_in_delta(0.25, @material.roughness_factor)
  end

  def test_normal
    @material.normal_texture = "#{ENV['TEST_RESOURCES']}/texture_map.png" # enabling needs a texture
    @material.normal_enabled = true
    assert(@material.normal_enabled?)
    @material.normal_scale = 2.0
    assert_in_delta(2.0, @material.normal_scale)
    assert_equal(Sketchup::Material::NORMAL_STYLE_DIRECTX, @material.normal_style = Sketchup::Material::NORMAL_STYLE_DIRECTX)
    assert_equal(Sketchup::Material::NORMAL_STYLE_DIRECTX, @material.normal_style)
    assert_raises(ArgumentError) { @material.normal_style = 9 }
  end

  def test_ambient_occlusion
    @material.ao_texture = "#{ENV['TEST_RESOURCES']}/texture_map.png" # enabling needs a texture
    @material.ao_enabled = true
    assert(@material.ao_enabled?)
    @material.ao_strength = 0.75
    assert_in_delta(0.75, @material.ao_strength)
  end

  def test_pbr_textures
    path = "#{ENV['TEST_RESOURCES']}/texture_map.png"
    %i[metallic_texture roughness_texture normal_texture ao_texture].each do |name|
      assert_nil(@material.public_send(name), name)
      assert_equal(path, @material.public_send("#{name}=", path))
      assert_instance_of(Sketchup::Texture, @material.public_send(name), name)
      assert_nil(@material.public_send("#{name}=", nil))
      assert_nil(@material.public_send(name), name)
    end
    assert_raises(ArgumentError) { @material.metallic_texture = '/does/not/exist.png' }
  end
end
