require 'minitest/autorun'
require 'sketchup'
require 'tmpdir'

TEST_RESOURCES = ENV.fetch('TEST_RESOURCES') { File.expand_path('../test-resources', __dir__) }

module ScriptupTestHelper
  def open_model(file = 'Untitled.skp')
    Sketchup.open_file(File.join(TEST_RESOURCES, file))
    Sketchup.active_model
  end

  # Methods from the RBS that the C API can't support; they must stay undefined.
  def assert_unsupported(object, *methods)
    methods.flatten.each { |m| refute_respond_to(object, m, "#{m} has no C API counterpart and should not be defined") }
  end

  def assert_point(expected, actual, delta = 1e-6)
    assert_equal(3, actual.to_a.size)
    expected.to_a.zip(actual.to_a).each { |e, a| assert_in_delta(e, a, delta) }
  end

  def add_square_face(entities, size = 10, z = 0)
    entities.add_face([0, 0, z], [size, 0, z], [size, size, z], [0, size, z])
  end
end

Minitest::Test.include(ScriptupTestHelper)
