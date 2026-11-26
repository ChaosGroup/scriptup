require_relative 'helper'

class TestSectionPlane < Minitest::Test
  def setup
    @model = open_model
    @section = @model.entities.add_section_plane([0, 0, 5], [0, 0, 1])
  end

  def test_class
    assert_kind_of(Sketchup::Drawingelement, @section)
  end

  def test_name_and_symbol
    assert_equal('Section', @section.name = 'Section')
    assert_equal('Section', @section.name)
    assert_equal('A', @section.symbol = 'A')
    assert_equal('A', @section.symbol)
    assert_raises(TypeError) { @section.name = 1 }
  end

  def test_plane
    plane = @section.get_plane
    assert_equal(4, plane.size)
    assert(Geom::Point3d.new(3, 3, 5).on_plane?(plane))
    assert_same(@section, @section.set_plane([[0, 0, 9], [0, 0, 1]]))
    assert(Geom::Point3d.new(3, 3, 9).on_plane?(@section.get_plane))
    assert_raises(ArgumentError) { @section.set_plane([1]) }
  end

  def test_activate
    refute(@section.active?)
    assert_same(@section, @section.activate)
    assert(@section.active?)
    assert_equal(@section, @model.entities.active_section_plane)
  end
end
