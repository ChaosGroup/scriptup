require 'yard'

module ScriptupRbs
  module YardLoader
    module_function

    def stubs_dir
      spec = Gem::Specification.find_by_name('sketchup-api-stubs')
      File.join(spec.gem_dir, 'lib', 'sketchup-api-stubs', 'stubs')
    end

    def stubs_version
      Gem::Specification.find_by_name('sketchup-api-stubs').version.to_s
    end

    def load(dir = stubs_dir)
      YARD::Registry.clear
      YARD::Logger.instance.level = Logger::ERROR
      YARD.parse(Dir["#{dir}/**/*.rb"].sort)
      YARD::Registry
    end
  end
end
