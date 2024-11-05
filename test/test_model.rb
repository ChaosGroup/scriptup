require 'minitest/autorun'
require 'sketchup'

class TestModel < Minitest::Test

  def setup
    Sketchup.open_file("#{ENV['TEST_RESOURCES']}/Untitled.skp")
  end

  def test_close
    assert_nil(Sketchup.active_model.close)
    assert_nil(Sketchup.active_model)
  end

end
