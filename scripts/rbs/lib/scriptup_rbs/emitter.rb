require 'rbs'

module ScriptupRbs
  class Emitter
    CORE_REOPEN = { 'Array' => '[unchecked out Elem]', 'String' => '', 'Numeric' => '', 'Object' => '' }.freeze

    attr_reader :report

    def initialize(registry, target, report)
      @registry = registry
      @target = target
      @report = report
      @translator = TypeTranslator.new(registry.all(:class, :module).map(&:path)) do |source|
        report.log(:untyped, "#{report.context}: #{source}")
      end
      @methods = MethodBuilder.new(@translator, target, report)
    end

    # Returns { 'sketchup/texture.rbs' => [RBS decls] }
    def build
      files = {}
      namespaces.each do |ns, version|
        source = ns.root? ? root_source(ns) : namespace_source(ns, version)
        next unless source

        path = self.class.file_for(ns.root? ? '_top_level' : ns.path)
        files[path] = parse(source, path)
      end
      files
    end

    def self.file_for(type_name)
      parts = type_name.delete_prefix('::').split('::').map do |p|
        p.gsub(/([A-Z]+)([A-Z][a-z])/, '\1_\2').gsub(/([a-z\d])([A-Z])/, '\1_\2').downcase
      end
      "#{parts.join('/')}.rbs"
    end

    private

    def parse(source, path)
      _, _, decls = RBS::Parser.parse_signature(RBS::Buffer.new(name: path, content: source))
      decls
    rescue RBS::ParsingError => e
      raise "#{path}: #{e.message}\n#{source}"
    end

    # Yields namespaces that survive the version filter, with their effective version.
    def namespaces
      versions = {}
      result = [[@registry.root, nil]]
      @registry.all(:class, :module).sort_by(&:path).each do |ns|
        parent = ns.namespace
        next if !parent.root? && !versions.key?(parent.path)

        version = version_of(ns) || versions[parent.path]
        next if version && version > @target

        versions[ns.path] = version
        result << [ns, version]
      end
      result
    end

    def version_of(object)
      tag = object.tag(:version)
      tag && Version.parse(tag.text, log: ->(msg) { @report.log(:version, "#{object.path}: #{msg}") })
    end

    def namespace_source(ns, version)
      header = if ns.type == :class
                 if CORE_REOPEN.key?(ns.path)
                   "class #{ns.path}#{CORE_REOPEN[ns.path]}"
                 else
                   sup = ns.superclass && @translator.resolve(ns.superclass.path, 'root')
                   sup && sup != 'Object' ? "class #{ns.path} < #{sup}" : "class #{ns.path}"
                 end
               else
                 "module #{ns.path}"
               end
      header = "%a{deprecated} #{header}" if ns.has_tag?(:deprecated)
      lines = [header]
      lines += mixins(ns)
      lines += constants(ns)
      lines += methods(ns, version)
      lines << 'end'
      lines.join("\n  ") + "\n"
    end

    def root_source(root)
      consts = root.children.select { |c| c.type == :constant }.map { |c| "#{c.name}: #{constant_type(c)}" }
      meths = methods(root, nil)
      out = consts.join("\n")
      out += "\nclass Object\n  #{meths.join("\n  ")}\nend" unless meths.empty?
      "#{out}\n"
    end

    def mixins(ns)
      lines = ns.mixins(:instance).map do |m|
        case m.path
        when 'Enumerable' then "include Enumerable[#{enumerable_element(ns)}]"
        else "include #{@translator.resolve(m.path, ns.path) || m.path}"
        end
      end
      lines + ns.mixins(:class).map { |m| "extend #{@translator.resolve(m.path, ns.path) || m.path}" }
    end

    def enumerable_element(ns)
      each = ns.meths(inherited: false, included: false).find { |m| m.name == :each && m.scope == :instance }
      tags = each && (each.tags(:yieldparam) + each.tags(:overload).flat_map { |o| o.tags(:yieldparam) })
      types = tags&.first&.types
      return 'untyped' unless types && tags.size == 1

      @translator.translate(types, ns.path)
    end

    def constants(ns)
      ns.children.select { |c| c.type == :constant }.map { |c| "#{c.name}: #{constant_type(c)}" }
    end

    def constant_type(_constant)
      'Integer'
    end

    def methods(ns, version)
      ns.children.select { |c| c.type == :method && !c.is_alias? }.filter_map do |m|
        @methods.build(m, version)
      end
    end
  end
end
