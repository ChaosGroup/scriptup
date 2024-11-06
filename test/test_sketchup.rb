require 'minitest/autorun'
require 'sketchup'

class TestSketchup < Minitest::Test

  def test_open_file
    status = Sketchup.open_file("#{ENV['TEST_RESOURCES']}/Untitled.skp")
    assert_equal(Sketchup::Model::LOAD_STATUS_SUCCESS, status)
  end

  def test_active_model
    Sketchup.active_model&.close
    assert_nil(Sketchup.active_model)
    Sketchup.open_file("#{ENV['TEST_RESOURCES']}/Untitled.skp")
    assert_instance_of(Sketchup::Model, Sketchup.active_model)
  end

end
