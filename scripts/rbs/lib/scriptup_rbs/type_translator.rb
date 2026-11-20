require 'strscan'

module ScriptupRbs
  class TypeTranslator
    CORE = %w[
      Array BasicObject Class Comparable Enumerable Exception Float Hash Integer IO Method Module
      NilClass Numeric Object Proc Range Regexp String Struct Symbol Time
    ].freeze

    LITERALS = { 'Boolean' => 'bool', 'true' => 'true', 'false' => 'false', 'nil' => 'nil',
                 'Object' => 'untyped', 'void' => 'void' }.freeze

    class ParseError < StandardError; end

    def initialize(known_paths, &on_untyped)
      @known = known_paths.to_set
      @on_untyped = on_untyped || ->(_) {}
    end

    # types: YARD tag types (already split on top-level commas); namespace: YARD path for name lookup
    def translate(types, namespace = 'root')
      return untyped('<none>') if types.nil? || types.empty?

      union(types.map { |t| render(parse(t), namespace) })
    rescue ParseError
      untyped(types.join(', '))
    end

    # Element type for a *splat param documented as Array<X>
    def translate_splat(types, namespace = 'root')
      return untyped('<none>') if types.nil? || types.empty?

      nodes = types.flat_map do |t|
        node = parse(t)
        node[0] == 'Array' && node[1] == :angle ? node[2] : [node]
      end
      union(nodes.map { |n| render(n, namespace) })
    rescue ParseError
      untyped(types.join(', '))
    end

    def resolve(name, namespace)
      name = name.delete_prefix('::')
      return name if @known.include?(name) && name.include?('::')

      scopes = namespace.to_s == 'root' ? [] : namespace.to_s.split('::')
      until scopes.empty?
        candidate = "#{scopes.join('::')}::#{name}"
        return candidate if @known.include?(candidate)

        scopes.pop
      end
      return name if @known.include?(name) || CORE.include?(name)

      nil
    end

    private

    def untyped(source)
      @on_untyped.call(source)
      'untyped'
    end

    def parse(text)
      s = StringScanner.new(text.strip)
      node = parse_type(s)
      s.skip(/\s*/)
      raise ParseError, text unless s.eos?

      node
    end

    def parse_type(s)
      s.skip(/\s*/)
      name = s.scan(/#?[\w:]+/) or raise ParseError
      s.skip(/\s*/)
      if s.scan(/</)
        args = parse_list(s, />/)
        [name, :angle, args]
      elsif s.scan(/\(/)
        [name, :paren, parse_list(s, /\)/)]
      elsif s.scan(/\{/)
        keys = parse_list(s, /=>/)
        [name, :brace, [keys, parse_list(s, /\}/)]]
      else
        [name, nil, []]
      end
    end

    def parse_list(s, close)
      items = []
      loop do
        items << parse_type(s)
        s.skip(/\s*/)
        next if s.scan(/,/)
        break if s.scan(close)

        raise ParseError
      end
      items
    end

    def render(node, namespace)
      name, kind, args = node
      return LITERALS[name] if LITERALS.key?(name)
      return untyped(name) if name.start_with?('#')

      case name
      when 'Array'
        case kind
        when :paren then "[#{args.map { |a| render(a, namespace) }.join(', ')}]"
        when :angle then "Array[#{union(args.map { |a| render(a, namespace) })}]"
        else 'Array[untyped]'
        end
      when 'Hash'
        case kind
        when :brace
          keys, values = args
          "Hash[#{union(keys.map { |a| render(a, namespace) })}, #{union(values.map { |a| render(a, namespace) })}]"
        when :angle
          return 'Hash[untyped, untyped]' unless args.size == 2

          "Hash[#{render(args[0], namespace)}, #{render(args[1], namespace)}]"
        else 'Hash[untyped, untyped]'
        end
      else
        resolve(name, namespace) || untyped(name)
      end
    end

    def union(parts)
      parts = parts.uniq
      return 'untyped' if parts.include?('untyped')

      parts << 'bool' if parts.include?('true') && parts.include?('false')
      parts = (parts - %w[true false]).uniq if parts.include?('bool')
      nilable = parts.delete('nil')
      return 'nil' if parts.empty?

      if nilable
        return "#{parts.first}?" if parts.size == 1 && !parts.first.include?(' ')

        return "(#{parts.join(' | ')})?"
      end
      parts.size == 1 ? parts.first : "(#{parts.join(' | ')})"
    end
  end
end
