require_relative 'helper'

class TestPages < Minitest::Test
  # No C API: animation timing and observers.
  UNSUPPORTED = %i[show_frame_at slideshow_time add_observer remove_observer].freeze

  def setup
    @model = open_model
    @pages = @model.pages
  end

  def test_unsupported
    assert_unsupported(@pages, UNSUPPORTED)
    refute_respond_to(Sketchup::Pages, :add_frame_change_observer)
  end

  def test_class
    assert_instance_of(Sketchup::Pages, @pages)
    assert_kind_of(Enumerable, @pages)
    assert_equal(@model, @pages.parent)
  end

  def test_add
    assert_equal(0, @pages.count)
    page = @pages.add('Front')
    assert_instance_of(Sketchup::Page, page)
    assert_equal('Front', page.name)
    assert_equal(1, @pages.length)
    assert_equal('Front (1)', @pages.add('Front').name, 'names are made unique')
    first = @pages.add('First', PAGE_USE_ALL, 0)
    assert_equal(first, @pages[0])
    assert_instance_of(Sketchup::Page, @pages.add)
  end

  def test_index_and_each
    page = @pages.add('Side')
    assert_equal(page, @pages['Side'])
    assert_equal(page, @pages[0])
    assert_nil(@pages['Missing'])
    seen = []
    assert_nil(@pages.each { |p| seen << p })
    assert_equal([page], seen)
    assert_equal(1, @pages.size)
  end

  def test_erase
    page = @pages.add('Gone')
    assert(@pages.erase(page))
    assert_equal(0, @pages.count)
    assert_raises(TypeError) { @pages.erase('x') }
  end

  def test_reorder
    a = @pages.add('A')
    b = @pages.add('B')
    assert_nil(@pages.reorder(b, 0))
    assert_equal([b, a], @pages.to_a)
  end

  def test_selected_page
    a = @pages.add('A')
    b = @pages.add('B')
    assert(@pages.selected_page = b)
    assert_equal(b, @pages.selected_page)
    refute_equal(a, @pages.selected_page)
  end

  def test_unique_name
    assert_equal('Plan', @pages.unique_name('Plan'))
    @pages.add('Plan')
    assert_equal('Plan (1)', @pages.unique_name('Plan'))
  end

  def test_add_matchphoto_page
    page = @pages.add_matchphoto_page(File.join(TEST_RESOURCES, 'texture_map.png'))
    assert_instance_of(Sketchup::Page, page)
    camera = Sketchup::Camera.new([0, -10, 5], [0, 0, 0], [0, 0, 1])
    assert_equal('Match', @pages.add_matchphoto_page(File.join(TEST_RESOURCES, 'texture_map.png'), camera, 'Match').name)
  end
end
