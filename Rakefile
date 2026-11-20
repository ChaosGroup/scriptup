require 'bundler/setup'
require 'rake/extensiontask'
require 'rake/testtask'
require 'rake/clean'
require 'json'
require_relative 'package'

Bundler.setup

prefix = RbConfig::CONFIG['prefix']
ENV['PATH'] = "#{prefix}/msys64/usr/bin;#{prefix}/msys64/mingw64/bin;#{ENV['PATH']}"

CLEAN.include 'lib/sketchup.bundle', 'lib/sketchup.so', 'Makefile', 'tmp', '*.o', 'sketchup.bundle', 'sketchup.so'

Rake::ExtensionTask.new 'sketchup' do |ext|
  ext.lib_dir = 'lib'
  ext.ext_dir = 'ext'
end

task :download do
  JSON.parse(File.read("#{__dir__}/packages.json"), symbolize_names: true).each { |package_name, package_info|
    path = "#{__dir__}/#{package_name}"
    next if File.exist?(path)
    puts "Fetching #{package_name} => #{package_info}"
    Package.new(**package_info).download(path)
  }
end

task :compile do
  ENV['SCRIPTUP_DEVELOPMENT'] = '1'
  sh "ruby #{__dir__}/ext/extconf.rb"
  sh 'make'
end

Rake::TestTask.new do |t|
  ENV['TEST_RESOURCES'] = "#{__dir__}/test-resources"
  t.libs << 'lib'
  t.test_files = FileList['test/**/test_*.rb']
end

task default: [:download, :compile, :test]

namespace :rbs do
  rbs_bundle = lambda do |cmd|
    Bundler.with_unbundled_env do
      sh({ 'BUNDLE_GEMFILE' => "#{__dir__}/scripts/rbs/Gemfile" }, "bundle exec #{cmd}", chdir: __dir__)
    end
  end

  desc 'Install the RBS tooling bundle (scripts/rbs/Gemfile)'
  task :setup do
    Bundler.with_unbundled_env { sh({ 'BUNDLE_GEMFILE' => "#{__dir__}/scripts/rbs/Gemfile" }, 'bundle install') }
  end

  desc 'Generate the Ruby API RBS into sig/ (reports in workspace/rbs/reports)'
  task :generate do
    rbs_bundle.call('ruby scripts/rbs/bin/generate')
  end

  desc 'Validate the generated RBS'
  task validate: :generate do
    rbs_bundle.call('rbs -I sig validate')
  end

  desc 'Compare the compiled extension with sig/ (CLASSES="Sketchup::Face ..." to filter)'
  task coverage: :compile do
    rbs_bundle.call("ruby scripts/rbs/bin/coverage #{ENV.fetch('CLASSES', '')}")
  end

  desc 'Run the RBS generator tests'
  task :test do
    rbs_bundle.call(%q(ruby -e 'Dir["scripts/rbs/test/test_*.rb"].each { |f| require File.expand_path(f) }'))
  end
end

desc 'Generate, validate and test RBS'
task rbs: %w[rbs:generate rbs:validate rbs:test]
