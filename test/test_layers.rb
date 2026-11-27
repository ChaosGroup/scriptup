require_relative 'helper'

class TestLayers < Minitest::Test
  UNSUPPORTED = %i[add_observer remove_observer].freeze # no C API

  def setup
    @model = open_model
    @layers = @model.layers
  end

  def test_unsupported
    assert_unsupported(@layers, UNSUPPORTED)
  end

  def test_model_accessors
    assert_instance_of(Sketchup::Layers, @layers)
    assert_instance_of(Sketchup::Layers, @model.tags)
    assert_kind_of(Enumerable, @layers)
  end

  def test_counts
    before = @layers.length
    assert_equal(before, @layers.size)
    assert_equal(before, @layers.count)
    @layers.add('New')
    assert_equal(before + 1, @layers.length)
    assert_equal(before + 1, @layers.count_layers)
  end

  def test_each
    @layers.add('One')
    names = []
    @layers.each { |l| assert_instance_of(Sketchup::Layer, l); names << l.name }
    assert_includes(names, 'One')
    assert_equal(@layers.to_a, @layers.layers)
    seen = []
    @layers.each_layer { |l| seen << l }
    assert_equal(@layers.layers, seen)
  end

  def test_index
    layer = @layers.add('Index')
    assert_equal(layer, @layers['Index'])
    assert_equal(layer, @layers.at('Index'))
    assert_instance_of(Sketchup::Layer, @layers[0])
    assert_nil(@layers['Missing'])
    assert_nil(@layers[999])
  end

  def test_add
    layer = @layers.add('Roof')
    assert_instance_of(Sketchup::Layer, layer)
    assert_equal(layer, @layers.add('Roof'), 'adding an existing name returns it')
    assert_raises(TypeError) { @layers.add(1) }
  end

  def test_folders
    assert_equal(0, @layers.count_folders)
    folder = @layers.add_folder('Group A')
    assert_instance_of(Sketchup::LayerFolder, folder)
    assert_equal([folder], @layers.folders)
    assert_equal(1, @layers.count_folders)
    seen = []
    @layers.each_folder { |f| seen << f }
    assert_equal([folder], seen)
    assert_nil(@layers.remove_folder(folder))
    assert_equal(0, @layers.count_folders)
  end

  def test_remove
    @layers.add('Temp')
    assert(@layers.remove('Temp'))
    assert_nil(@layers['Temp'])
    layer = @layers.add('Temp2')
    assert(@layers.remove(layer))
    refute(@layers.remove('Missing'))
    @layers.add('Temp3')
    assert_raises(NotImplementedError) { @layers.remove('Temp3', true) }
  end

  def test_purge
    @layers.add('Unused')
    assert_operator(@layers.purge_unused, :>=, 1)
    assert_nil(@layers['Unused'])
    @layers.add_folder('Empty')
    assert_equal(1, @layers.purge_unused_folders)
  end

  def test_unique_name
    assert_equal('Fresh', @layers.unique_name('Fresh'))
    @layers.add('Tag')
    assert_equal('Tag1', @layers.unique_name('Tag'))
    assert_instance_of(String, @layers.unique_name)
  end
end
