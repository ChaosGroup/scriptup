module ScriptupRbs
  class Report
    attr_accessor :context

    def initialize
      @entries = Hash.new { |h, k| h[k] = [] }
    end

    def log(kind, line)
      @entries[kind] << line
    end

    def [](kind)
      @entries[kind]
    end

    def write(kind, path, header)
      File.write(path, ([header] + @entries[kind].uniq).join("\n") + "\n")
    end
  end
end
