require_relative 'helper'

class TestVector2d < Minitest::Test
  def setup
    @vector = Geom::Vector2d.new(3, 4)
  end

  def test_initialize
    assert_equal([0.0, 0.0], Geom::Vector2d.new.to_a)
    assert_equal([3.0, 4.0], @vector.to_a)
    assert_equal([1.0, 2.0], Geom::Vector2d.new([1, 2]).to_a)
    assert_equal([3.0, 4.0], Geom::Vector2d.new(@vector).to_a)
    assert_raises(ArgumentError) { Geom::Vector2d.new([1, 2, 3]) }
    assert_raises(TypeError) { Geom::Vector2d.new('x') }
    assert_raises(ArgumentError) { Geom::Vector2d.new(1, 2, 3) }
  end

  def test_components
    assert_instance_of(Float, @vector.x)
    assert_equal(3.0, @vector.x)
    assert_equal(4.0, @vector.y)
    assert_equal(1, @vector.x = 1)
    assert_equal(2, @vector.y = 2)
    assert_equal([1.0, 2.0], @vector.to_a)
  end

  def test_index
    assert_instance_of(Float, @vector[0])
    assert_equal(4.0, @vector[1])
    assert_equal(7, @vector[0] = 7)
    assert_equal(7.0, @vector.x)
    assert_raises(IndexError) { @vector[2] }
  end

  def test_clone_and_set!
    copy = @vector.clone
    copy.x = 9
    assert_equal(3.0, @vector.x)
    assert_same(@vector, @vector.set!(1, 1))
    assert_equal([1.0, 1.0], @vector.to_a)
    @vector.set!([2, 3])
    assert_equal([2.0, 3.0], @vector.to_a)
  end

  def test_arithmetic
    assert_equal([4.0, 5.0], (@vector + [1, 1]).to_a)
    assert_equal([2.0, 3.0], (@vector - Geom::Vector2d.new(1, 1)).to_a)
  end

  def test_dot_and_cross
    assert_equal(25.0, @vector % @vector)
    assert_equal(3.0, @vector.dot([1, 0]))
    assert_equal(1.0, Geom::Vector2d.new(1, 0) * [0, 1])
    assert_equal(-4.0, @vector.cross([1, 0]))
  end

  def test_angle_between
    assert_in_delta(Math::PI / 2, Geom::Vector2d.new(1, 0).angle_between([0, 1]))
  end

  def test_comparison
    assert(@vector == [3, 4])
    assert(@vector == Geom::Vector2d.new(3, 4))
    refute(@vector == [3, 5])
    refute(@vector == 'x')
  end

  def test_predicates
    x = Geom::Vector2d.new(1, 0)
    assert(x.parallel?([-1, 0]))
    assert(x.perpendicular?([0, 2]))
    assert(x.same_direction?([5, 0]))
    refute(x.same_direction?([-5, 0]))
    assert(x.unit_vector?)
    refute(@vector.unit_vector?)
    assert(@vector.valid?)
    refute(Geom::Vector2d.new.valid?)
  end

  def test_length
    assert_instance_of(Length, @vector.length)
    assert_equal(5.0, @vector.length.to_f)
    assert_equal(10, @vector.length = 10)
    assert_equal([6.0, 8.0], @vector.to_a)
    assert_raises(ArgumentError) { Geom::Vector2d.new.length = 1 }
  end

  def test_normalize_and_reverse
    assert(@vector.normalize.unit_vector?)
    refute(@vector.unit_vector?)
    assert_same(@vector, @vector.normalize!)
    assert(@vector.unit_vector?)
    assert_equal([-1.0, -2.0], Geom::Vector2d.new(1, 2).reverse.to_a)
    vector = Geom::Vector2d.new(1, 2)
    assert_same(vector, vector.reverse!)
    assert_equal([-1.0, -2.0], vector.to_a)
  end

  def test_strings
    assert_equal('(3, 4)', @vector.to_s)
    assert_equal('Vector2d(3, 4)', @vector.inspect)
  end
end
