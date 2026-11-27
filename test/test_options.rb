require_relative 'helper'

class TestOptions < Minitest::Test
  UNSUPPORTED = %i[add_observer remove_observer].freeze # no C API

  def setup
    @model = open_model
    @manager = @model.options
  end

  def test_manager
    assert_instance_of(Sketchup::OptionsManager, @manager)
    assert_kind_of(Enumerable, @manager)
    assert_includes(@manager.keys, 'UnitsOptions')
    assert_equal(@manager.keys.size, @manager.count)
    assert_equal(@manager.keys.size, @manager.length)
    assert_equal(@manager.keys.size, @manager.size)
    providers = []
    assert_nil(@manager.each { |p| providers << p })
    providers.each { |p| assert_instance_of(Sketchup::OptionsProvider, p) }
    assert_equal('UnitsOptions', @manager['UnitsOptions'].name)
    assert_equal(@manager.keys.first, @manager[0].name)
    assert_nil(@manager['Nope'])
  end

  def test_provider
    units = @manager['UnitsOptions']
    assert_unsupported(units, UNSUPPORTED)
    assert_kind_of(Enumerable, units)
    assert_includes(units.keys, 'LengthUnit')
    assert(units.has_key?('LengthUnit'))
    assert(units.key?('LengthUnit'))
    refute(units.key?('Nope'))
    assert_equal(units.keys.size, units.count)
    assert_equal(units.keys.size, units.size)
    assert_equal(units.keys.size, units.length)
    assert_instance_of(Integer, units['LengthUnit'])
    assert_equal(units[units.keys.first], units[0])
  end

  def test_provider_iteration
    units = @manager['UnitsOptions']
    pairs = []
    assert_nil(units.each { |k, v| pairs << [k, v] })
    assert_equal(units.keys, pairs.map(&:first))
    values = []
    units.each_value { |v| values << v }
    assert_equal(pairs.map(&:last), values)
    keys = []
    units.each_key { |k| keys << k }
    assert_equal(units.keys, keys)
  end

  def test_provider_set
    units = @manager['UnitsOptions']
    assert_equal(Length::Meter, units['LengthUnit'] = Length::Meter)
    assert_equal(Length::Meter, units['LengthUnit'])
    assert_raises(ArgumentError) { units['Nope'] = 1 }
  end
end
