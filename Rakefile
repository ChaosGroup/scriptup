require 'bundler/setup'
require 'rake/extensiontask'
require 'rake/testtask'
require 'rake/clean'
require 'tmpdir'
require_relative 'sketchup_sdk'

Bundler.setup

prefix = RbConfig::CONFIG['prefix']
ENV['PATH'] = "#{prefix}/msys64/usr/bin;#{prefix}/msys64/mingw64/bin;#{ENV['PATH']}"

CLEAN.include 'lib/sketchup.bundle', 'lib/sketchup.so', 'Makefile', 'tmp', '*.o', 'sketchup.bundle', 'sketchup.so'

Rake::ExtensionTask.new 'sketchup' do |ext|
  ext.lib_dir = 'lib'
  ext.ext_dir = 'ext'
end

task :download do
  mac_sdk = "#{__dir__}/sketchup-sdk-mac"
  win_sdk = "#{__dir__}/sketchup-sdk-win"
  next if File.exist?(mac_sdk) && File.exist?(win_sdk)

  puts 'Downloading SketchUp SDK'
  Dir.mktmpdir { |tmpdir|
    win_package = SketchUpSDK.download(:win, tmpdir)
    mac_package = SketchUpSDK.download(:mac, tmpdir)
    SketchUpSDK.unzip(win_package, win_sdk)
    SketchUpSDK.unzip(mac_package, mac_sdk)
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
