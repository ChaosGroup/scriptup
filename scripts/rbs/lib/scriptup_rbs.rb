require 'fileutils'
require 'set'

module ScriptupRbs
  ROOT = File.expand_path('../../..', __dir__)
  SCRIPTS_DIR = File.join(ROOT, 'scripts', 'rbs')
  WORKSPACE_DIR = File.join(ROOT, 'workspace', 'rbs')

  module_function

  TARGET_VERSION = Gem::Version.new('2026.0')

  def target_version
    TARGET_VERSION
  end

  def sig_dir
    File.join(ROOT, 'sig')
  end

  def write_files(dir, files, header)
    FileUtils.rm_rf(dir)
    files.sort.each do |path, decls|
      next if decls.empty?

      out = File.join(dir, path)
      FileUtils.mkdir_p(File.dirname(out))
      File.open(out, 'w') do |f|
        f.puts header, ''
        RBS::Writer.new(out: f).write(decls)
      end
    end
  end
end

require_relative 'scriptup_rbs/version'
require_relative 'scriptup_rbs/report'
require_relative 'scriptup_rbs/yard_loader'
require_relative 'scriptup_rbs/type_translator'
require_relative 'scriptup_rbs/method_builder'
require_relative 'scriptup_rbs/emitter'
require_relative 'scriptup_rbs/overrides'
