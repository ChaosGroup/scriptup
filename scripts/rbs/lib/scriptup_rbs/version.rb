module ScriptupRbs
  module Version
    FIXUPS = { '20201.0' => '2020.1' }.freeze

    module_function

    def parse(text, log: nil)
      return nil if text.nil?

      m = text.strip.match(/\A(?:SketchUp|LayOut)\s+(\d+(?:\.\d+)*)(?:\s+M(\d+))?\z/)
      unless m
        log&.call("unparseable @version: #{text.inspect}")
        return nil
      end
      number = FIXUPS.fetch(m[1], m[1])
      number += '.0' unless number.include?('.')
      number += ".#{m[2]}" if m[2]
      Gem::Version.new(number)
    end
  end
end
