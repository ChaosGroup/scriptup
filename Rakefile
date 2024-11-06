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
