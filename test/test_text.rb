require_relative 'helper'

class TestText < Minitest::Test
  UNSUPPORTED = %i[line_weight line_weight=].freeze # no C API

  def setup
    @model = open_model
    @text = @model.entities.add_text('Hello', [1, 2, 3], [5, 5, 5])
  end

  def test_unsupported
    assert_unsupported(@text, UNSUPPORTED)
  end

  def test_constants
    assert_equal([0, 1, 2], [ALeaderNone, ALeaderView, ALeaderModel])
    assert_equal([0, 1, 2, 3, 4], %i[ARROW_NONE ARROW_SLASH ARROW_DOT ARROW_CLOSED ARROW_OPEN].map { |c| Sketchup::Dimension.const_get(c) })
  end

  def test_text
    assert_equal('Hello', @text.text)
    assert_equal('World', @text.text = 'World')
    assert_equal('World', @text.text)
    assert_same(@text, @text.set_text('Again'))
    assert_equal('Again', @text.text)
    assert_raises(TypeError) { @text.text = 1 }
  end

  def test_point_and_vector
    assert_equal(Geom::Point3d.new(1, 2, 3), @text.point)
    assert_equal([4, 4, 4], @text.point = [4, 4, 4])
    assert_equal(Geom::Point3d.new(4, 4, 4), @text.point)
    assert_equal([5.0, 5.0, 5.0], @text.vector.to_a)
    @text.vector = Geom::Vector3d.new(1, 0, 0)
    assert_equal([1.0, 0.0, 0.0], @text.vector.to_a)
  end

  def test_leader
    @text.leader_type = ALeaderModel
    assert_equal(ALeaderModel, @text.leader_type)
    assert(@text.has_leader?)
    assert(@text.display_leader?)
    assert_equal(false, @text.display_leader = false)
    refute(@text.display_leader?)
    assert_equal(ALeaderNone, @text.leader_type)
    @text.display_leader = true
    assert_equal(ALeaderView, @text.leader_type)
    assert_raises(ArgumentError) { @text.leader_type = 7 }
  end

  def test_arrow_type
    assert_equal(Sketchup::Dimension::ARROW_DOT, @text.arrow_type = Sketchup::Dimension::ARROW_DOT)
    assert_equal(Sketchup::Dimension::ARROW_DOT, @text.arrow_type)
    assert_raises(ArgumentError) { @text.arrow_type = 9 }
  end

  def test_attached_to
    assert_nil(@text.attached_to)
    group = @model.entities.add_group
    face = add_square_face(group.entities)
    path = Sketchup::InstancePath.new([group, face])
    @text.attached_to = [path, Geom::Point3d.new(5, 5, 0)]
    path_out, point = @text.attached_to
    assert_instance_of(Sketchup::InstancePath, path_out)
    assert_equal(path, path_out)
    assert_equal(Geom::Point3d.new(5, 5, 0), point)
    assert_raises(ArgumentError) { @text.attached_to = [path] }
  end
end
