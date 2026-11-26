require_relative 'helper'

class TestBehavior < Minitest::Test
  def setup
    @model = open_model
    @definition = @model.definitions.add('Behaving')
    @behavior = @definition.behavior
  end

  def test_class
    assert_instance_of(Sketchup::Behavior, @behavior)
  end

  def test_always_face_camera
    refute(@behavior.always_face_camera?)
    assert_equal(true, @behavior.always_face_camera = true)
    assert(@definition.behavior.always_face_camera?, 'changes are written to the definition')
  end

  def test_cuts_opening
    refute(@behavior.cuts_opening?)
    @behavior.cuts_opening = true
    assert(@behavior.cuts_opening?)
  end

  def test_shadows_face_sun
    @behavior.shadows_face_sun = true
    assert(@behavior.shadows_face_sun?)
    @behavior.shadows_face_sun = false
    refute(@behavior.shadows_face_sun?)
  end

  def test_no_scale_mask
    assert_equal(0, @behavior.no_scale_mask?)
    assert_equal(5, @behavior.no_scale_mask = 5)
    assert_equal(5, @behavior.no_scale_mask?)
    @behavior.no_scale_mask = 0xFF
    assert_equal(0x7F, @behavior.no_scale_mask?)
  end

  def test_is2d_and_snapto
    refute(@behavior.is2d?)
    assert_equal(0, @behavior.snapto)
    @behavior.is2d = true
    assert(@behavior.is2d?)
    assert_equal(2, @behavior.snapto = 2)
    assert_equal(2, @behavior.snapto)
    assert(@behavior.is2d?)
    @behavior.is2d = false
    refute(@behavior.is2d?)
    assert_raises(ArgumentError) { @behavior.snapto = 9 }
  end
end
