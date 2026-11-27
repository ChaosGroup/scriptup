require_relative 'helper'

class TestConstructionPoint < Minitest::Test
  def setup
    @model = open_model
    @cpoint = @model.entities.add_cpoint([1, 2, 3])
  end

  def test_class
    assert_kind_of(Sketchup::Drawingelement, @cpoint)
    assert_equal('ConstructionPoint', @cpoint.class.name.split('::').last)
  end

  def test_position
    assert_instance_of(Geom::Point3d, @cpoint.position)
    assert_equal(Geom::Point3d.new(1, 2, 3), @cpoint.position)
  end
end
