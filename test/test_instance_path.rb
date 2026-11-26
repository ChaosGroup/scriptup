require_relative 'helper'

class TestInstancePath < Minitest::Test
  def setup
    @model = open_model
    @outer = @model.entities.add_group
    @outer.transformation = Geom::Transformation.translation([10, 0, 0])
    @inner = @outer.entities.add_group
    @inner.transformation = Geom::Transformation.translation([0, 5, 0])
    @face = add_square_face(@inner.entities)
    @path = Sketchup::InstancePath.new([@outer, @inner, @face])
  end

  def test_initialize
    assert_kind_of(Enumerable, @path)
    assert_raises(TypeError) { Sketchup::InstancePath.new('x') }
    assert_raises(ArgumentError) { Sketchup::InstancePath.new([@face, @outer]) }
  end

  def test_length
    assert_equal(3, @path.length)
    assert_equal(3, @path.size)
  end

  def test_to_a_and_index
    assert_equal([@outer, @inner, @face], @path.to_a)
    assert_equal(@inner, @path[1])
    assert_raises(IndexError) { @path[5] }
  end

  def test_each
    seen = []
    assert_nil(@path.each { |e| seen << e })
    assert_equal([@outer, @inner, @face], seen)
  end

  def test_root_and_leaf
    assert_equal(@outer, @path.root)
    assert_equal(@face, @path.leaf)
    assert_nil(Sketchup::InstancePath.new([]).root)
  end

  def test_empty_and_valid
    refute(@path.empty?)
    assert(@path.valid?)
    assert(Sketchup::InstancePath.new([]).empty?)
  end

  def test_include?
    assert(@path.include?(@inner))
    refute(@path.include?(@model.entities.add_group))
    refute(@path.include?('x'))
  end

  def test_persistent_id_path
    assert_match(/\A\d+(\.\d+)*\z/, @path.persistent_id_path)
  end

  def test_transformation
    assert_equal(Geom::Point3d.new(10, 5, 0), @path.transformation.origin)
    assert_equal(Geom::Point3d.new(10, 0, 0), @path.transformation(0).origin)
    assert_raises(IndexError) { @path.transformation(9) }
  end

  def test_equality
    assert(@path == Sketchup::InstancePath.new([@outer, @inner, @face]))
    refute(@path == Sketchup::InstancePath.new([@outer, @inner]))
    refute(@path == 'x')
  end
end
