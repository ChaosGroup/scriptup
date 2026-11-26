require_relative 'helper'

class TestModel < Minitest::Test
  # Need the SketchUp application (operations, views, tools, export/import, ...) or have no C API.
  UNSUPPORTED = %i[abort_operation commit_operation start_operation active_path active_path= active_view add_note
                   add_observer remove_observer close_active edit_transform export import get_datum set_datum
                   list_datums get_product_family mipmapping= mipmapping? modified? overlays place_component raytest
                   save_thumbnail select_tool selection tools latlong_to_point point_to_latlong point_to_utm
                   utm_to_point find_entity_by_id tags=].freeze

  def setup
    @model = open_model
  end

  def test_unsupported
    assert_unsupported(@model, UNSUPPORTED)
  end

  def test_close
    assert_nil(Sketchup.active_model.close)
    assert_nil(Sketchup.active_model)
  end

  def test_equality
    assert(@model == Sketchup.active_model)
    assert_equal(@model.hash, Sketchup.active_model.hash)
    refute(@model == 'x')
  end

  def test_collections
    assert_instance_of(Sketchup::Entities, @model.entities)
    assert_equal(@model.entities, @model.active_entities)
    assert_instance_of(Sketchup::Materials, @model.materials)
    assert_instance_of(Sketchup::DefinitionList, @model.definitions)
    assert_instance_of(Sketchup::Layers, @model.layers)
    assert_instance_of(Sketchup::AttributeDictionaries, @model.attribute_dictionaries)
  end

  def test_active_layer
    layer = @model.active_layer
    assert_instance_of(Sketchup::Layer, layer)
    walls = @model.layers.add('Walls')
    assert_equal(walls, @model.active_layer = walls)
    assert_equal(walls, @model.active_layer)
    @model.active_layer = layer.name
    assert_equal(layer, @model.active_layer)
    assert_raises(ArgumentError) { @model.active_layer = 'Missing' }
  end

  def test_active_section_planes
    assert_equal([], @model.active_section_planes)
  end

  def test_metadata
    assert_instance_of(String, @model.title)
    assert_equal('Untitled', File.basename(@model.path, '.skp'))
    refute_empty(@model.guid)
    assert_equal('My model', @model.name = 'My model')
    assert_equal('My model', @model.name)
    assert_equal('About', @model.description = 'About')
    assert_equal('About', @model.description)
    assert_includes([true, false], @model.georeferenced?)
    assert(@model.valid?)
  end

  def test_bounds_and_statistics
    assert_instance_of(Geom::BoundingBox, @model.bounds)
    faces = @model.number_faces
    add_square_face(@model.entities)
    assert_equal(faces + 1, @model.number_faces)
  end

  def test_attributes
    assert_instance_of(Sketchup::AttributeDictionary, @model.attribute_dictionary('GeoReference'))
    assert_nil(@model.attribute_dictionary('Missing'))
    assert_instance_of(Sketchup::AttributeDictionary, @model.attribute_dictionary('Created', true))
    assert_equal(42, @model.set_attribute('Test', 'answer', 42))
    assert_equal(42, @model.get_attribute('Test', 'answer'))
    assert_equal('x', @model.get_attribute('Test', 'missing', 'x'))
  end

  def test_find_entity_by_persistent_id
    definition = @model.definitions['Heather']
    assert_equal(definition, @model.find_entity_by_persistent_id(definition.persistent_id))
    assert_equal([definition, nil], @model.find_entity_by_persistent_id(definition.persistent_id, 999_999_999))
    assert_raises(ArgumentError) { @model.find_entity_by_persistent_id }
  end

  def test_instance_paths
    group = @model.entities.add_group
    face = add_square_face(group.entities)
    path = @model.instance_path_from_pid_path("#{group.persistent_id}.#{face.persistent_id}")
    assert_instance_of(Sketchup::InstancePath, path)
    assert_equal(face, path.leaf)
    assert(@model.drawing_element_visible?(path))
    face.hidden = true
    refute(@model.drawing_element_visible?([group, face]))
    assert_raises(ArgumentError) { @model.instance_path_from_pid_path('1.2.3') }
  end

  def test_behavior
    behavior = @model.behavior
    assert_instance_of(Sketchup::Behavior, behavior)
    behavior.no_scale_mask = 3
    assert_equal(3, @model.behavior.no_scale_mask?)
  end

  def test_save
    Dir.mktmpdir do |dir|
      path = File.join(dir, 'saved.skp')
      assert(@model.save(path))
      assert(File.exist?(path))
      copy = File.join(dir, 'copy.skp')
      assert(@model.save_copy(copy))
      assert(File.exist?(copy))
    end
  end
end
