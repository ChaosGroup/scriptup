require_relative 'helper'

class TestLength < Minitest::Test
  def setup
    open_model
  end

  def test_class
    assert_kind_of(Numeric, 1.to_l)
    refute_kind_of(Float, 1.to_l)
    assert_raises(NoMethodError) { Length.new }
  end

  def test_constants
    assert_equal(0, Length::Architectural)
    assert_equal(1, Length::Decimal)
    assert_equal(2, Length::Engineering)
    assert_equal(3, Length::Fractional)
    assert_equal([0, 1, 2, 3, 4, 5], [Length::Inches, Length::Feet, Length::Millimeter, Length::Centimeter, Length::Meter, Length::Yard])
    assert_equal(5, Length::SquareYard)
    assert_equal(7, Length::USGallon)
  end

  def test_to_f
    assert_instance_of(Float, 12.to_l.to_f)
    assert_equal(12.0, 12.to_l.to_f)
  end

  def test_to_l
    length = 5.to_l
    assert_same(length, length.to_l)
  end

  def test_comparison
    assert(11.to_l < 12.to_l)
    refute(12.to_l < 11.to_l)
    assert(11.to_l <= 11.to_l)
    assert(12.to_l > 11.to_l)
    assert(12.to_l >= 12.to_l)
    assert_equal(-1, 20.to_l <=> 30.to_l)
    assert_equal(0, 20.to_l <=> 20.to_l)
    assert_equal(1, 30.to_l <=> 20.to_l)
    assert_nil(20.to_l <=> 'x')
  end

  def test_equality_uses_tolerance
    assert(1.to_l == 1.0001.to_l)
    refute(1.to_l == 1.01.to_l)
    assert(1.to_l == 1)
    refute(1.to_l == 'x')
  end

  def test_arithmetic_returns_float
    assert_equal(3.0, 1.to_l + 2)
    assert_equal(3.0, 2 + 1.to_l)
    assert_equal(-1.0, 1.to_l - 2)
    assert_equal(4.0, 2.to_l * 2)
    assert_equal(1.0, 2.to_l / 2)
    assert_equal(-2.0, -(2.to_l))
    assert_instance_of(Float, 1.to_l + 1)
  end

  def test_to_s
    assert_instance_of(String, 1.m.to_s)
    assert_in_delta(1.m.to_f, 1.m.to_s.to_l.to_f, 0.1) # rounded to model precision
  end

  def test_inspect
    assert_equal('12.0', 12.to_l.inspect)
  end

  def test_hash_and_eql
    assert(1.to_l.eql?(1.to_l))
    refute(1.to_l.eql?(1.0))
    assert_equal(1.to_l.hash, 1.to_l.hash)
  end

  def test_numeric_units
    assert_instance_of(Length, 1.inch)
    assert_in_delta(12.0, 1.feet.to_f)
    assert_in_delta(36.0, 1.yard.to_f)
    assert_in_delta(63_360.0, 1.mile.to_f)
    assert_in_delta(1 / 25.4, 1.mm.to_f)
    assert_in_delta(1 / 2.54, 1.cm.to_f)
    assert_in_delta(100 / 2.54, 1.m.to_f)
    assert_in_delta(100_000 / 2.54, 1.km.to_f)
    assert_instance_of(Length, 2.5.to_l)
  end

  def test_numeric_conversions
    assert_in_delta(2.54, 1.to_cm)
    assert_in_delta(1.0, 12.to_feet)
    assert_in_delta(12.0, 12.to_inch)
    assert_in_delta(0.0254, 1.to_m)
    assert_in_delta(25.4, 1.to_mm)
    assert_in_delta(1.0, 36.to_yard)
    assert_in_delta(1.0, 63_360.to_mile)
    assert_in_delta(1.0, (100_000 / 2.54).to_km)
    assert_in_delta(1.0, 1.m.to_m)
  end

  def test_angles
    assert_in_delta(Math::PI / 2, 90.degrees)
    assert_in_delta(90.0, (Math::PI / 2).radians)
  end

  def test_string_to_l
    assert_instance_of(Length, '1"'.to_l)
    assert_in_delta(1.0, '1"'.to_l.to_f)
    assert_in_delta(12.0, "1'".to_l.to_f)
    assert_raises(ArgumentError) { 'not a length'.to_l }
  end
end
