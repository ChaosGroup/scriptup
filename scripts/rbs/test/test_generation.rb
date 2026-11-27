require_relative 'helper'

class TestGeneration < Minitest::Test
  def self.registry
    @registry ||= ScriptupRbs::YardLoader.load
  end

  def build(target, overrides: nil)
    report = ScriptupRbs::Report.new
    files = ScriptupRbs::Emitter.new(self.class.registry, Gem::Version.new(target), report).build
    ScriptupRbs::Overrides.new(overrides, Gem::Version.new(target), report).apply(files) if overrides
    [files, report]
  end

  def method_in(files, type, name)
    decl = files.values.flatten.find { |d| d.name.to_s == type }
    decl&.members&.find { |m| m.is_a?(RBS::AST::Members::MethodDefinition) && m.name == name }
  end

  def test_overloads_filtered_by_version
    old, = build('2018.0')
    new, = build('2026.0')
    assert_equal 1, method_in(old, 'Sketchup::Entities', :add_text).overloads.size
    assert_equal 3, method_in(new, 'Sketchup::Entities', :add_text).overloads.size
  end

  def test_methods_filtered_by_version
    old, = build('2024.0')
    new, = build('2026.0')
    assert_nil method_in(old, 'Sketchup::Entities', :add_snap)
    refute_nil method_in(new, 'Sketchup::Entities', :add_snap)
    assert_nil method_in(new, 'Sketchup::AppObserver', :onCloseModel), '2027.0 method leaked into 2026.0'
  end

  def test_overrides_with_version_annotations
    dir = File.join(__dir__, 'fixtures', 'overrides')
    old, report = build('2019.0', overrides: dir)
    new, = build('2026.0', overrides: dir)

    assert_equal 'Symbol', method_in(old, 'Sketchup::Texture', :filename).overloads.first.method_type.type.return_type.to_s
    assert_equal 'String', method_in(new, 'Sketchup::Texture', :filename).overloads.first.method_type.type.return_type.to_s
    assert_equal 'Integer', method_in(old, 'Sketchup::Texture', :width).overloads.first.method_type.type.return_type.to_s
    assert_equal 'Float', method_in(new, 'Sketchup::Texture', :width).overloads.first.method_type.type.return_type.to_s
    assert_nil method_in(new, 'Sketchup::Texture', :height)
    assert(report[:overrides].any? { |l| l.include?('no_such_method') })
    assert_empty method_in(old, 'Sketchup::Texture', :filename).annotations
  end

  def test_full_output_validates
    Dir.mktmpdir do |dir|
      files, = build('2026.0', overrides: File.join(ScriptupRbs::SCRIPTS_DIR, 'overrides'))
      ScriptupRbs.write_files(dir, files, '# test')
      loader = RBS::EnvironmentLoader.new
      loader.add(path: Pathname(dir))
      env = RBS::Environment.from_loader(loader).resolve_type_names
      builder = RBS::DefinitionBuilder.new(env: env)
      env.class_decls.each_key do |name|
        next unless name.to_s.start_with?('::Sketchup', '::Geom')

        builder.build_instance(name)
        builder.build_singleton(name)
      end
    end
  end
end
