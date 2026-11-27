module ScriptupRbs
  class MethodBuilder
    PARAM_NAME = /\A[a-z_][a-zA-Z0-9_]*\z/
    RBS_KEYWORDS = %w[
      alias attr_accessor attr_reader attr_writer bool bot class def end extend false include instance
      interface module nil out prepend private public self singleton super top true type unchecked untyped void
    ].freeze

    def initialize(translator, target, report)
      @translator = translator
      @target = target
      @report = report
    end

    # Returns the RBS member source for a YARD method, or nil when nothing survives the version filter.
    def build(meth, inherited_version)
      method_version = version_of(meth) || inherited_version
      return nil if newer?(method_version)

      sources = meth.tags(:overload)
      sources = [meth] if sources.empty?
      signatures = sources.filter_map do |source|
        next if source != meth && newer?(version_of(source) || method_version)

        signature(meth, source)
      end.uniq
      return nil if signatures.empty?

      prefix = meth.scope == :class ? 'self.' : ''
      annotations = meth.has_tag?(:deprecated) ? "%a{deprecated} " : ''
      visibility = meth.namespace.root? ? 'private ' : ''
      "#{annotations}#{visibility}def #{prefix}#{method_name(meth)}: #{signatures.join("\n    | ")}"
    end

    private

    def version_of(object)
      tag = object.tag(:version)
      tag && Version.parse(tag.text, log: ->(msg) { @report.log(:version, "#{object.path}: #{msg}") })
    end

    def newer?(version)
      version && version > @target
    end

    def method_name(meth)
      name = meth.name.to_s
      RBS_KEYWORDS.include?(name) ? "`#{name}`" : name
    end

    def location(meth, what)
      "#{meth.path} #{what}"
    end

    def signature(meth, source)
      @report.context = meth.path
      ns = meth.namespace.root? ? 'root' : meth.namespace.path
      param_tags = source.tags(:param).group_by { |t| t.name.to_s.delete_prefix('*').delete_prefix('&').delete_suffix(':') }
      option_tags = source.tags(:option).map { |t| t.name.to_s }

      required = []
      optional = []
      trailing = []
      rest = nil
      keywords = []
      block = nil
      positional_types = []

      source.parameters.each do |raw_name, default|
        name = raw_name.to_s
        bare = name.delete_prefix('**').delete_prefix('*').delete_prefix('&').delete_suffix(':')
        tag = param_tags[bare]&.first
        type = param_type(meth, tag, bare, ns, splat: name.start_with?('*') && !name.start_with?('**'))
        type = 'Hash[Symbol, untyped]' if type == 'Hash[untyped, untyped]' && option_tags.include?(bare)
        label = param_label(bare)
        positional_types << type unless name.start_with?('*', '&') || name.end_with?(':')

        if name.start_with?('&')
          block = true
        elsif name.start_with?('**')
          keywords << "**#{type}#{label}"
        elsif name.start_with?('*')
          rest = "*#{type}#{label}"
        elsif name.end_with?(':')
          keywords << "#{default.nil? ? '' : '?'}#{bare}: #{type}"
        elsif !default.nil?
          optional << "?#{type}#{label}"
        elsif optional.empty? && rest.nil?
          required << "#{type}#{label}"
        else
          trailing << "#{type}#{label}"
        end
      end

      params = (required + optional + (rest ? [rest] : []) + trailing + keywords).join(', ')
      block_sig = block_signature(meth, source, ns, block)
      "(#{params})#{block_sig} -> #{return_type(meth, source, ns, positional_types)}"
    end

    def param_label(name)
      PARAM_NAME.match?(name) && !RBS_KEYWORDS.include?(name) ? " #{name}" : ''
    end

    def param_type(meth, tag, name, ns, splat: false)
      return untyped(location(meth, "param #{name}"), '<no @param>') unless tag
      return untyped(location(meth, "param #{name}"), tag.text.to_s.lines.first.to_s.strip) unless tag.types

      splat ? @translator.translate_splat(tag.types, ns) : @translator.translate(tag.types, ns)
    end

    def block_signature(meth, source, ns, has_block_param)
      yield_tags = %i[yield yieldparam yieldreturn].flat_map { |t| source.tags(t) }
      yield_tags = %i[yield yieldparam yieldreturn].flat_map { |t| meth.tags(t) } if yield_tags.empty? && source != meth
      return '' if yield_tags.empty? && !has_block_param

      yieldparams = yield_tags.select { |t| t.tag_name == 'yieldparam' }
      args = if yieldparams.any?
               yieldparams.map do |t|
                 type = t.types ? @translator.translate(t.types, ns) : 'untyped'
                 "#{type}#{param_label(t.name.to_s)}"
               end.join(', ')
             else
               names = yield_tags.find { |t| t.tag_name == 'yield' }&.types
               names ? names.map { |n| "untyped#{param_label(n)}" }.join(', ') : '*untyped'
             end
      yieldreturn = yield_tags.find { |t| t.tag_name == 'yieldreturn' }
      ret = yieldreturn&.types ? @translator.translate(yieldreturn.types, ns) : 'untyped'
      " ?{ (#{args}) -> #{ret} }"
    end

    def return_type(meth, source, ns, positional)
      return 'void' if meth.name == :initialize

      tag = source.tag(:return)
      if tag.nil? || tag.types.nil?
        return positional.first if meth.name.to_s.end_with?('=') && positional.size == 1

        return untyped(location(meth, 'return'), tag ? tag.text.to_s.lines.first.to_s.strip : '<no @return>')
      end
      @translator.translate(tag.types, ns)
    end

    def untyped(where, text)
      @report.log(:untyped, "#{where}: #{text}")
      'untyped'
    end
  end
end
