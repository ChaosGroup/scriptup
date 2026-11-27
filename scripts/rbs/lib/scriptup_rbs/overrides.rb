require 'rbs'

module ScriptupRbs
  # Merges hand-written RBS from overrides/ into generated decls.
  # Annotations: %a{sketchup_rbs:remove}, %a{sketchup_rbs:since=X}, %a{sketchup_rbs:until=X}
  class Overrides
    PREFIX = 'sketchup_rbs:'.freeze
    CONTAINER = [RBS::AST::Declarations::Class, RBS::AST::Declarations::Module].freeze

    def initialize(dir, target, report)
      @dir = dir
      @target = target
      @report = report
    end

    def apply(files)
      Dir["#{@dir}/**/*.rbs"].sort.each do |path|
        _, _, decls = RBS::Parser.parse_signature(RBS::Buffer.new(name: path, content: File.read(path)))
        decls.each { |decl| merge_decl(files, decl) }
      end
      files
    end

    private

    def merge_decl(files, decl)
      return unless applies?(decl.annotations)

      file, existing = find(files, decl)
      if remove?(decl.annotations)
        files[file].delete(existing) if existing
        return
      end

      decl = clean(decl)
      unless existing
        target_file = files.key?(file) ? file : Emitter.file_for(decl.name.to_s)
        (files[target_file] ||= []) << strip_members(decl)
        return
      end

      if CONTAINER.include?(decl.class)
        decl.members.each { |member| merge_member(existing, member) }
      else
        files[file][files[file].index(existing)] = decl
      end
    end

    def merge_member(container, member)
      return unless applies?(member.annotations)

      index = container.members.index { |m| same_member?(m, member) }
      if remove?(member.annotations)
        index ? container.members.delete_at(index) : stale(container, member)
        return
      end

      member = clean(member)
      if index
        container.members[index] = member
      else
        stale(container, member) if member.is_a?(RBS::AST::Members::MethodDefinition)
        container.members << member
      end
    end

    def strip_members(decl)
      return decl unless CONTAINER.include?(decl.class)

      members = decl.members.select { |m| !m.respond_to?(:annotations) || applies?(m.annotations) }.map { |m| clean(m) }
      decl.update(members: members)
    end

    def same_member?(a, b)
      return false unless a.class == b.class

      case a
      when RBS::AST::Members::MethodDefinition then a.name == b.name && a.kind == b.kind
      when RBS::AST::Declarations::TypeAlias, RBS::AST::Declarations::Constant then a.name == b.name
      when RBS::AST::Members::Include, RBS::AST::Members::Extend then a.name == b.name
      else false
      end
    end

    def find(files, decl)
      files.each do |file, decls|
        found = decls.find { |d| d.class == decl.class && d.name == decl.name }
        return [file, found] if found
      end
      [Emitter.file_for(decl.name.to_s), nil]
    end

    def stale(container, member)
      @report.log(:overrides, "#{container.name}: override #{member.name} has no generated counterpart")
    end

    def directives(annotations)
      annotations.map(&:string).select { |s| s.start_with?(PREFIX) }.map { |s| s.delete_prefix(PREFIX) }
    end

    def applies?(annotations)
      directives(annotations).all? do |d|
        case d
        when /\Asince=(.+)\z/ then @target >= Gem::Version.new(Regexp.last_match(1))
        when /\Auntil=(.+)\z/ then @target < Gem::Version.new(Regexp.last_match(1))
        else true
        end
      end
    end

    def remove?(annotations)
      directives(annotations).include?('remove')
    end

    def clean(node)
      return node unless node.respond_to?(:annotations) && directives(node.annotations).any?

      node.update(annotations: node.annotations.reject { |a| a.string.start_with?(PREFIX) })
    end
  end
end
