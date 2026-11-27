require_relative 'helper'

class TestTypeTranslator < Minitest::Test
  SNAPSHOT = File.join(__dir__, 'fixtures', 'types.snapshot')
  TAGS = %w[param return yieldparam yieldreturn].freeze

  def setup
    @untyped = []
    @t = ScriptupRbs::TypeTranslator.new(%w[Geom Geom::Point3d Geom::Vector3d Sketchup Sketchup::Color Length]) { |s| @untyped << s }
  end

  def test_basics
    assert_equal 'bool', @t.translate(['Boolean'])
    assert_equal 'untyped', @t.translate(['Object'])
    assert_equal 'Integer?', @t.translate(%w[Integer nil])
    assert_equal '(Integer | false)', @t.translate(%w[Integer false])
    assert_equal 'bool', @t.translate(%w[true false])
    assert_equal 'Array[Geom::Point3d]', @t.translate(['Array<Geom::Point3d>'])
    assert_equal 'Array[(Integer | String)]', @t.translate(['Array<Integer, String>'])
    assert_equal '[ Float, Float ]'.delete(' '), @t.translate(['Array(Float, Float)']).delete(' ')
    assert_equal 'Hash[Symbol, untyped]', @t.translate(['Hash{Symbol => Object}'])
    assert_equal 'Hash[(Symbol | String), (Integer | bool)]', @t.translate(['Hash{Symbol, String => Integer, Boolean}'])
    assert_equal 'Hash[untyped, untyped]', @t.translate(['Hash'])
  end

  def test_resolves_relative_names
    assert_equal 'Geom::Point3d', @t.translate(['Point3d'], 'Geom')
    assert_equal 'Sketchup::Color', @t.translate(['Color'], 'Sketchup::Material')
    assert_equal 'Length', @t.translate(['Length'], 'Geom')
  end

  def test_unknown_logs_untyped
    assert_equal 'untyped', @t.translate(['#frameChange'])
    assert_equal 'untyped', @t.translate(['Nope'])
    assert_equal 'untyped', @t.translate(nil)
    assert_equal 3, @untyped.size
  end

  def test_splat_unwraps_array
    assert_equal 'Geom::Point3d', @t.translate_splat(['Array<Geom::Point3d>'])
  end

  # Every distinct YARD type string in the stubs must translate to parseable RBS; output is snapshotted.
  def test_all_stub_types
    registry = ScriptupRbs::YardLoader.load
    t = ScriptupRbs::TypeTranslator.new(registry.all(:class, :module).map(&:path))
    types = registry.all(:method).flat_map do |m|
      (m.tags + m.tags(:overload).flat_map(&:tags)).filter_map do |tag|
        [tag.types, m.namespace.root? ? 'root' : m.namespace.path] if tag.types && TAGS.include?(tag.tag_name)
      end
    end.uniq.sort_by(&:to_s)
    lines = types.map do |ty, ns|
      rbs = t.translate(ty, ns)
      RBS::Parser.parse_type(rbs)
      "#{ns} #{ty.join(', ')} => #{rbs}"
    end.uniq
    File.write(SNAPSHOT, lines.join("\n") + "\n") if ENV['UPDATE_SNAPSHOT'] || !File.exist?(SNAPSHOT)
    assert_equal File.read(SNAPSHOT), lines.join("\n") + "\n", 'run with UPDATE_SNAPSHOT=1 to accept changes'
  end
end
