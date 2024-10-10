Gem::Specification.new do |spec|
  spec.name          = 'scriptup'
  spec.version       = '2024.0.0'
  spec.authors       = ['Noel Warren']
  spec.email         = ['noel.warren@chaos.com']
  spec.summary       = 'Ruby bindings for the SketchUp C API'
  spec.description   = 'This project provides ruby bindings around the SketchUp C API. They are designed to mimmic the SketchUp ruby api for use in Sketchup extension tests outside the SketchUp application.  You can however use them for general purpouse scripting.'
  spec.homepage      = 'https://git.chaosgroup.com/sketchup/sketchup-ruby-bindings'
  spec.license       = 'MIT'

  spec.files         = Dir[
    'README.md',
    'LICENSE.txt',
    'lib/sketchup.rb',
    'ext/*',
    'sketchup-sdk-*/**/*',
  ].reject { |f| File.symlink?(f) }
  spec.require_paths = ['lib']
  spec.extensions    = ['ext/extconf.rb']
  spec.required_ruby_version = '>= 2.5'

  spec.add_development_dependency 'bundler', '~> 2.4.10'
  spec.add_development_dependency 'rake', '~> 13.0'
end
