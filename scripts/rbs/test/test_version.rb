require_relative 'helper'

class TestVersion < Minitest::Test
  V = ScriptupRbs::Version

  def test_parse
    assert_equal Gem::Version.new('6.0'), V.parse('SketchUp 6.0')
    assert_equal Gem::Version.new('8.0.2'), V.parse('SketchUp 8.0 M2')
    assert_equal Gem::Version.new('2019.0'), V.parse('SketchUp 2019')
    assert_equal Gem::Version.new('2016.0.1'), V.parse('SketchUp 2016 M1')
    assert_equal Gem::Version.new('2018.0'), V.parse('LayOut 2018')
    assert_equal Gem::Version.new('2020.1'), V.parse('SketchUp 20201.0')
  end

  def test_unparseable
    logged = []
    assert_nil V.parse('soon', log: ->(m) { logged << m })
    assert_equal 1, logged.size
  end
end
