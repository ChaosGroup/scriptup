require 'mkmf'

MAC = RUBY_PLATFORM =~ /darwin/
WIN = !MAC

extension_name = 'sketchup'
sdk_path = "#{__dir__}/../sketchup-sdk-#{MAC ? 'mac' : 'win'}"

# Work around for how framework headers are structured
if MAC && !File.exist?("#{sdk_path}/headers/SketchUpAPI")
  FileUtils.mkdir_p("#{sdk_path}/headers")
  FileUtils.symlink('../SketchUpAPI.framework/Headers', "#{sdk_path}/headers/SketchUpAPI")
end

# Work around for fact gems can't handle symlinks
if MAC
  framework = "#{sdk_path}/SketchUpAPI.framework"
  FileUtils.symlink('Versions/A/SketchUpAPI', "#{framework}/SketchUpAPI") unless File.exist?("#{framework}/SketchUpAPI")
  FileUtils.symlink('Versions/A/Resources', "#{framework}/Resources") unless File.exist?("#{framework}/Resources")
  FileUtils.symlink('Versions/A/Headers', "#{framework}/Headers") unless File.exist?("#{framework}/Headers")
  FileUtils.symlink('A', "#{framework}/Versions/Current") unless File.exist?("#{framework}/Versions/Current")
end

# rubocop:disable Style/GlobalVars
$INCFLAGS << " -I#{sdk_path}/headers"
$CFLAGS << " #{'-Werror' if MAC } -O0 " if ENV['SCRIPTUP_DEVELOPMENT']

if MAC
  $DLDFLAGS.gsub!('-Wl,-multiply_defined,suppress', '')
  $DLDFLAGS << ' -Wl'
  $DLDFLAGS << " -F#{sdk_path} -framework SketchUpAPI"
  $DLDFLAGS << " -Wl,-rpath,#{sdk_path}"
else
  $DLDFLAGS << " -L#{sdk_path}/binaries/sketchup/x64 -lSketchUpAPI"
end
# rubocop:enable Style/GlobalVars

create_makefile(extension_name)
