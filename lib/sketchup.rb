case RUBY_PLATFORM
when /darwin/
    require "#{__dir__}/sketchup.bundle"
when /linux/
    raise "Unsupported platform for SketchUpAPI"
else # assuming windows
    require 'fiddle'
    Fiddle.dlopen("#{__dir__}/../sketchup-sdk-win/binaries/sketchup/x64/SketchUpCommonPreferences.dll")
    Fiddle.dlopen("#{__dir__}/../sketchup-sdk-win/binaries/sketchup/x64/SketchUpAPI.dll")
    require "#{__dir__}/sketchup.so"
end
    