require_relative 'helper'

class TestMaterialsList < Minitest::Test
  # No C API: the current (UI) material, usage-based purging and observers.
  UNSUPPORTED = %i[current current= purge_unused add_observer remove_observer].freeze

  def setup
    Sketchup.open_file("#{ENV['TEST_RESOURCES']}/Untitled.skp")
    @materials = Sketchup.active_model.materials
  end

  def test_object
    assert_kind_of(Enumerable, @materials)
  end

  def test_get
    assert_raises(IndexError) {
      Sketchup.active_model.materials[19]
    }
    assert_raises(TypeError) {
      Sketchup.active_model.materials[Object.new]
    }
    assert_equal(Sketchup.active_model.materials['Heather_Shirt'].name, 'Heather_Shirt')
    assert_equal(Sketchup.active_model.materials[1].name, 'Heather_Shirt')
  end

  def test_each
    counter = 0
    assert_equal(@materials.each { |material|
      assert_instance_of(Sketchup::Material, material)
      counter += 1
    }, @materials)
    assert_equal(19, counter)
  end

  def test_unsupported
    assert_unsupported(@materials, UNSUPPORTED)
  end

  def test_counts
    assert_equal(19, @materials.length)
    assert_equal(19, @materials.size)
    assert_equal(19, @materials.count)
  end

  def test_at
    assert_equal(@materials[1], @materials.at(1))
    assert_equal('Heather_Shirt', @materials.at('Heather_Shirt').name)
  end

  def test_add
    material = @materials.add('Brick')
    assert_instance_of(Sketchup::Material, material)
    assert_equal('Brick', material.name)
    assert_equal(20, @materials.length)
    assert_equal('Brick1', @materials.add('Brick').name, 'names are made unique')
    assert_raises(TypeError) { @materials.add(1) }
  end

  def test_unique_name
    assert_equal('Fresh', @materials.unique_name('Fresh'))
    refute_equal('Heather_Shirt', @materials.unique_name('Heather_Shirt'))
  end

  def test_load_and_remove
    Dir.mktmpdir do |dir|
      path = File.join(dir, 'to_load.skm')
      saved = @materials.add('ToLoad')
      saved.save_as(path)
      assert(@materials.remove(saved))
      assert_equal(19, @materials.length)
      loaded = @materials.load(path)
      assert_instance_of(Sketchup::Material, loaded)
      assert_equal(20, @materials.length)
      assert(@materials.remove(loaded))
      assert_equal(19, @materials.length)
    end
    assert_raises(ArgumentError) { @materials.load('/does/not/exist.skm') }
    assert_raises(TypeError) { @materials.remove('x') }
  end
end
