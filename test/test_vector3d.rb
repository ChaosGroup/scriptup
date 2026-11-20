require_relative 'helper'

class TestVector3d < Minitest::Test
  def setup
    @vector = Geom::Vector3d.new(1, 2, 3)
  end

  def test_initialize
    assert_equal([0.0, 0.0, 0.0], Geom::Vector3d.new.to_a)
    assert_equal([1.0, 2.0, 3.0], @vector.to_a)
    assert_equal([1.0, 2.0, 3.0], Geom::Vector3d.new([1, 2, 3]).to_a)
    assert_equal([1.0, 2.0, 0.0], Geom::Vector3d.new([1, 2]).to_a)
    assert_equal([1.0, 2.0, 3.0], Geom::Vector3d.new(@vector).to_a)
    assert_raises(ArgumentError) { Geom::Vector3d.new(1, 2) }
    assert_raises(ArgumentError) { Geom::Vector3d.new([1]) }
    assert_raises(TypeError) { Geom::Vector3d.new('x') }
    assert_raises(TypeError) { Geom::Vector3d.new(1, 'x', 3) }
  end

  def test_linear_combination
    x = Geom::Vector3d.new(1, 0, 0)
    y = Geom::Vector3d.new(0, 1, 0)
    z = Geom::Vector3d.new(0, 0, 1)
    assert_equal([0.5, 0.5, 0.0], Geom::Vector3d.linear_combination(0.5, x, 0.5, y).to_a)
    assert_equal([1.0, 2.0, 3.0], Geom::Vector3d.linear_combination(1, x, 2, y, 3, z).to_a)
    assert_raises(ArgumentError) { Geom::Vector3d.linear_combination(1, x) }
  end

  def test_components
    assert_instance_of(Float, @vector.x)
    assert_equal(1.0, @vector.x)
    assert_equal(2.0, @vector.y)
    assert_equal(3.0, @vector.z)
    assert_equal(5, @vector.x = 5)
    assert_equal(6, @vector.y = 6)
    assert_equal(7, @vector.z = 7)
    assert_equal([5.0, 6.0, 7.0], @vector.to_a)
  end

  def test_index
    assert_instance_of(Length, @vector[0])
    assert_equal(3.0, @vector[2].to_f)
    assert_equal(9, @vector[1] = 9)
    assert_equal(9.0, @vector.y)
    assert_raises(IndexError) { @vector[3] }
    assert_raises(IndexError) { @vector[-1] = 1 }
  end

  def test_clone
    copy = @vector.clone
    refute_same(@vector, copy)
    copy.x = 10
    assert_equal(1.0, @vector.x)
  end

  def test_set!
    assert_same(@vector, @vector.set!(4, 5, 6))
    assert_equal([4.0, 5.0, 6.0], @vector.to_a)
    @vector.set!([7, 8, 9])
    assert_equal([7.0, 8.0, 9.0], @vector.to_a)
    @vector.set!(Geom::Vector3d.new(1, 1, 1))
    assert_equal([1.0, 1.0, 1.0], @vector.to_a)
  end

  def test_arithmetic
    assert_equal([2.0, 4.0, 6.0], (@vector + @vector).to_a)
    assert_equal([2.0, 3.0, 4.0], (@vector + [1, 1, 1]).to_a)
    assert_equal([0.0, 1.0, 2.0], (@vector - [1, 1, 1]).to_a)
  end

  def test_cross_and_dot
    x = Geom::Vector3d.new(1, 0, 0)
    y = Geom::Vector3d.new(0, 1, 0)
    assert_equal([0.0, 0.0, 1.0], (x * y).to_a)
    assert_equal([0.0, 0.0, 1.0], x.cross([0, 1, 0]).to_a)
    assert_equal(14.0, @vector % @vector)
    assert_equal(0.0, x.dot(y))
  end

  def test_comparison
    assert(Geom::Vector3d.new(1, 2, 3) == @vector)
    assert(@vector == [1, 2, 3])
    refute(@vector == [1, 2, 4])
    refute(@vector == 'x')
    assert(Geom::Vector3d.new(0, 0, 0) < @vector)
  end

  def test_angle_between
    assert_in_delta(Math::PI / 2, Geom::Vector3d.new(1, 0, 0).angle_between([0, 1, 0]))
  end

  def test_predicates
    x = Geom::Vector3d.new(1, 0, 0)
    assert(x.parallel?([-2, 0, 0]))
    assert(x.perpendicular?([0, 1, 0]))
    assert(x.samedirection?([3, 0, 0]))
    refute(x.samedirection?([-3, 0, 0]))
    assert(x.unitvector?)
    refute(@vector.unitvector?)
    assert(@vector.valid?)
    refute(Geom::Vector3d.new.valid?)
  end

  def test_length
    assert_instance_of(Length, @vector.length)
    assert_in_delta(Math.sqrt(14), @vector.length.to_f)
    assert_equal(2, @vector.length = 2)
    assert_in_delta(2.0, @vector.length.to_f)
    assert_raises(ArgumentError) { Geom::Vector3d.new.length = 1 }
  end

  def test_normalize
    normal = @vector.normalize
    assert(normal.unitvector?)
    refute(@vector.unitvector?)
    assert_same(@vector, @vector.normalize!)
    assert(@vector.unitvector?)
  end

  def test_reverse
    assert_equal([-1.0, -2.0, -3.0], @vector.reverse.to_a)
    assert_equal([1.0, 2.0, 3.0], @vector.to_a)
    assert_same(@vector, @vector.reverse!)
    assert_equal([-1.0, -2.0, -3.0], @vector.to_a)
  end

  def test_axes
    axes = Geom::Vector3d.new(0, 0, 5).axes
    assert_equal(3, axes.size)
    axes.each { |axis| assert(axis.unitvector?) }
    assert(axes[0].perpendicular?(axes[1]))
    assert(axes[2].samedirection?([0, 0, 1]))
  end

  def test_strings
    assert_equal('(1, 2, 3)', @vector.to_s)
    assert_equal('Vector3d(1, 2, 3)', @vector.inspect)
    assert_equal('(0.5, 0, 0)', Geom::Vector3d.new(0.5, 0, 0).to_s)
  end
end
