require_relative 'helper'

class TestSketchup < Minitest::Test
  # No C API outside SketchUp: the app, UI, preferences, extensions, undo, version/edition queries, etc.
  UNSUPPORTED = %i[add_observer remove_observer app_name break_edges= break_edges? debug_mode= debug_mode?
                   display_name_from_action extensions find_support_file find_support_files fix_shadow_strings=
                   fix_shadow_strings? focus format_angle format_degrees get_datfile_info get_i18n_datfile_info
                   get_locale get_resource_path get_shortcuts install_from_archive is_64bit? is_online is_pro?
                   is_valid_filename? load os_language platform plugins_disabled= plugins_disabled? quit
                   read_default redo register_extension register_importer require resize_viewport save_thumbnail
                   send_action send_to_layout set_status_text status_text= temp_dir template template=
                   template_dir undo vcb_label= vcb_value= version version_number write_default].freeze

  def test_unsupported
    assert_unsupported(Sketchup, UNSUPPORTED)
  end

  def test_open_file
    status = Sketchup.open_file(File.join(TEST_RESOURCES, 'Untitled.skp'))
    assert_equal(Sketchup::Model::LOAD_STATUS_SUCCESS, status)
    assert_equal(true, Sketchup.open_file(File.join(TEST_RESOURCES, 'Untitled.skp'), with_status: false))
    assert_equal(false, Sketchup.open_file('/does/not/exist.skp'))
  end

  def test_active_model
    Sketchup.active_model&.close
    assert_nil(Sketchup.active_model)
    open_model
    assert_instance_of(Sketchup::Model, Sketchup.active_model)
  end

  def test_file_new
    open_model
    old = Sketchup.active_model
    assert_equal(Sketchup, Sketchup.file_new)
    refute_equal(old, Sketchup.active_model)
    assert_equal(0, Sketchup.active_model.entities.count)
  end

  def test_format_length
    open_model
    assert_instance_of(String, Sketchup.format_length(12))
    assert_in_delta(12.0, Sketchup.parse_length(Sketchup.format_length(12)), 0.1)
    assert_instance_of(String, Sketchup.format_length(12.3456, 3))
  end

  def test_format_area_and_volume
    open_model
    assert_instance_of(String, Sketchup.format_area(144))
    assert_instance_of(String, Sketchup.format_volume(1728))
  end

  def test_parse_length
    open_model
    assert_in_delta(12.0, Sketchup.parse_length("1'"))
    assert_nil(Sketchup.parse_length('not a length'))
  end
end
