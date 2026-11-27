require_relative 'helper'

class TestPage < Minitest::Test
  # No C API: animation timing and the use_style flag.
  UNSUPPORTED = %i[delay_time delay_time= transition_time transition_time= use_style? use_style=].freeze

  def setup
    @model = open_model
    @page = @model.pages.add('Front')
  end

  def test_unsupported
    assert_unsupported(@page, UNSUPPORTED)
  end

  def test_constants
    assert_equal(0x0001, PAGE_USE_CAMERA)
    assert_equal(0x0fff, PAGE_USE_ALL)
    assert_equal(0x0ffe, PAGE_NO_CAMERA)
    assert_equal(0x0008, PAGE_USE_SKETCHCS)
  end

  def test_name_and_label
    assert_equal('Front', @page.name)
    assert_equal('Back', @page.name = 'Back')
    assert_equal('Back', @page.label)
  end

  def test_description
    assert_equal('Notes', @page.description = 'Notes')
    assert_equal('Notes', @page.description)
  end

  def test_owned_objects
    assert_instance_of(Sketchup::Camera, @page.camera)
    %i[use_axes= use_rendering_options= use_shadow_info=].each { |flag| @page.public_send(flag, true) }
    @page.update(PAGE_USE_ALL) # a page only stores what it was told to remember
    assert_instance_of(Sketchup::Axes, @page.axes)
    assert_instance_of(Sketchup::RenderingOptions, @page.rendering_options)
    assert_instance_of(Sketchup::ShadowInfo, @page.shadow_info)
    assert_kind_of(Sketchup::Style, @page.style) if @page.style
  end

  def test_flags
    %i[use_axes use_camera use_hidden_geometry use_hidden_layers use_hidden_objects use_rendering_options
       use_section_planes use_shadow_info include_in_animation].each do |flag|
      @page.public_send("#{flag}=", false)
      refute(@page.public_send("#{flag}?"), flag)
      @page.public_send("#{flag}=", true)
      assert(@page.public_send("#{flag}?"), flag)
    end
    assert_includes([true, false], @page.use_hidden?)
    assert_includes([true, false], @page.use_environment?)
  end

  def test_layer_visibility
    layer = @model.layers.add('Hidden here')
    assert_nil(@page.layers)
    assert_same(@page, @page.set_visibility(layer, false))
    assert_equal([layer], @page.layers)
    @page.set_visibility(layer, true)
    assert_nil(@page.layers)
    folder = @model.layers.add_folder('Folder')
    @page.set_visibility(folder, false)
    assert_equal([folder], @page.layer_folders)
  end

  def test_drawingelement_visibility
    face = add_square_face(@model.entities)
    assert(@page.get_drawingelement_visibility(face))
    assert(@page.set_drawingelement_visibility(face, false))
    refute(@page.get_drawingelement_visibility(face))
    assert_equal([face], @page.hidden_entities)
  end

  def test_section_planes_and_hidden
    assert_nil(@page.active_section_planes)
    assert_nil(@page.hidden_entities)
  end

  def test_update
    assert(@page.update)
    assert(@page.update(PAGE_USE_CAMERA))
  end
end
