require_relative 'helper'

class TestLayerFolder < Minitest::Test
  UNSUPPORTED = %i[visible_on_new_pages? visible_on_new_pages=].freeze # no C API

  def setup
    @model = open_model
    @folder = @model.layers.add_folder('Building')
    @layer = @model.layers.add('Walls')
  end

  def test_unsupported
    assert_unsupported(@folder, UNSUPPORTED)
  end

  def test_name
    assert_equal('Building', @folder.name)
    assert_equal('Site', @folder.name = 'Site')
    assert_equal('Site', @folder.name)
  end

  def test_comparable
    other = @model.layers.add_folder('Zones')
    assert_equal(-1, @folder <=> other)
    assert_nil(@folder <=> 1)
    assert(@folder == @model.layers.folders.first)
  end

  def test_visible
    assert(@folder.visible?)
    @folder.visible = false
    refute(@folder.visible?)
  end

  def test_layers
    assert_equal([], @folder.layers)
    assert_nil(@folder.add_layer(@layer))
    assert_equal([@layer], @folder.layers)
    assert_equal(1, @folder.count_layers)
    seen = []
    @folder.each_layer { |l| seen << l }
    assert_equal([@layer], seen)
    assert_nil(@folder.remove_layer(@layer))
    assert_equal(0, @folder.count_layers)
    assert_raises(ArgumentError) { @folder.remove_layer(@layer) }
  end

  def test_folders
    child = @folder.add_folder('Floor 1')
    assert_instance_of(Sketchup::LayerFolder, child)
    assert_equal([child], @folder.folders)
    assert_equal(1, @folder.count_folders)
    assert_equal(@folder, child.folder)
    seen = []
    @folder.each_folder { |f| seen << f }
    assert_equal([child], seen)
    assert_nil(@folder.remove_folder(child))
    assert_equal(0, @folder.count_folders)
  end

  def test_folder
    assert_nil(@folder.folder)
    parent = @model.layers.add_folder('Parent')
    assert_equal(parent, @folder.folder = parent)
    assert_equal(parent, @folder.folder)
    assert_raises(NotImplementedError) { @folder.folder = nil }
  end
end
