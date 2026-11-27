require_relative 'helper'

class TestEntities < Minitest::Test
  # No C API: 3D text, intersections, adding dimensions or snaps, and observers.
  UNSUPPORTED = %i[add_3d_text intersect_with add_dimension_linear add_dimension_radial add_snap
                   add_observer remove_observer].freeze

  def setup
    @model = open_model
    @entities = @model.entities
  end

  def test_unsupported
    assert_unsupported(@entities, UNSUPPORTED)
  end

  def test_enumerable
    assert_kind_of(Enumerable, @entities)
    count = 0
    assert_nil(@entities.each { |e| assert_kind_of(Sketchup::Entity, e); count += 1 })
    assert_equal(@entities.count, count)
  end

  def test_counts
    before = @entities.length
    assert_equal(before, @entities.size)
    assert_equal(before, @entities.count)
    @entities.add_line([0, 0, 0], [1, 0, 0])
    assert_equal(before + 1, @entities.length)
  end

  def test_index
    assert_kind_of(Sketchup::Entity, @entities[0])
    assert_equal(@entities[0], @entities.at(0))
    assert_nil(@entities[1000])
  end

  def test_model_and_parent
    assert_instance_of(Sketchup::Model, @entities.model)
    assert_instance_of(Sketchup::Model, @entities.parent)
    definition = @model.definitions['Heather']
    assert_equal(definition, definition.entities.parent)
    assert_instance_of(Sketchup::Model, definition.entities.model)
  end

  def test_add_line
    edge = @entities.add_line([0, 0, 0], Geom::Point3d.new(10, 0, 0))
    assert_instance_of(Sketchup::Edge, edge)
    assert(@entities.include?(edge))
    assert_nil(@entities.add_line([1, 1, 1], [1, 1, 1]))
    assert_raises(ArgumentError) { @entities.add_line([1], [2, 2, 2]) }
  end

  def test_add_edges
    edges = @entities.add_edges([0, 0, 0], [10, 0, 0], [10, 10, 0])
    assert_equal(2, edges.size)
    edges.each { |e| assert_instance_of(Sketchup::Edge, e) }
    assert_equal(3, @entities.add_edges([[0, 0, 1], [1, 0, 1], [1, 1, 1], [0, 1, 1]]).size)
    assert_raises(ArgumentError) { @entities.add_edges([0, 0, 0]) }
  end

  def test_add_face
    face = add_square_face(@entities)
    assert_instance_of(Sketchup::Face, face)
    assert(@entities.include?(face))
    face2 = @entities.add_face([[0, 0, 5], [5, 0, 5], [5, 5, 5]])
    assert_instance_of(Sketchup::Face, face2)
    edges = @entities.add_edges([0, 0, 9], [3, 0, 9], [3, 3, 9], [0, 0, 9])
    assert_instance_of(Sketchup::Face, @entities.add_face(edges))
    assert_raises(ArgumentError) { @entities.add_face([0, 0, 0], [1, 0, 0]) }
    assert_raises(ArgumentError) { @entities.add_face([0, 0, 0], [1, 0, 0], [2, 0, 0]) }
  end

  def test_add_circle
    edges = @entities.add_circle([0, 0, 0], [0, 0, 1], 5)
    assert_equal(24, edges.size)
    edges.each { |e| assert_instance_of(Sketchup::Edge, e) }
    assert_equal(8, @entities.add_circle([20, 0, 0], [0, 0, 1], 5, 8).size)
    assert_raises(ArgumentError) { @entities.add_circle([0, 0, 0], [0, 0, 1], -1) }
    assert_raises(ArgumentError) { @entities.add_circle([0, 0, 0], [0, 0, 1], 1, 2) }
  end

  def test_add_arc
    edges = @entities.add_arc([0, 0, 0], [1, 0, 0], [0, 0, 1], 5, 0, Math::PI / 2)
    refute_empty(edges)
    assert_equal(4, @entities.add_arc([0, 0, 0], [1, 0, 0], [0, 0, 1], 5, 0, Math::PI, 4).size)
    assert_raises(ArgumentError) { @entities.add_arc([0, 0, 0], [1, 0, 0], [0, 0, 1], 5, 0) }
  end

  def test_add_ngon
    edges = @entities.add_ngon([0, 0, 0], [0, 0, 1], 5, 6)
    assert_equal(6, edges.size)
    assert_equal(24, @entities.add_ngon([20, 0, 0], [0, 0, 1], 5).size)
    assert_raises(ArgumentError) { @entities.add_ngon([0, 0, 0], [0, 0, 1], 5, 2) }
  end

  def test_add_curve
    edges = @entities.add_curve([0, 0, 0], [1, 1, 0], [2, 0, 0], [3, 1, 0])
    assert_equal(3, edges.size)
    edges.each { |e| assert_instance_of(Sketchup::Edge, e) }
  end

  def test_add_cpoint
    cpoint = @entities.add_cpoint([1, 2, 3])
    assert_instance_of(Sketchup::ConstructionPoint, cpoint)
    assert(@entities.include?(cpoint))
  end

  def test_add_cline
    assert_instance_of(Sketchup::ConstructionLine, @entities.add_cline([0, 0, 0], [5, 0, 0]))
    assert_instance_of(Sketchup::ConstructionLine, @entities.add_cline([0, 0, 0], Geom::Vector3d.new(0, 1, 0)))
    assert_raises(NotImplementedError) { @entities.add_cline([0, 0, 0], [5, 0, 0], '-') }
  end

  def test_add_group
    group = @entities.add_group
    assert_instance_of(Sketchup::Group, group)
    assert(@entities.include?(group))
    edge = @entities.add_line([0, 0, 0], [1, 0, 0])
    assert_raises(NotImplementedError) { @entities.add_group(edge) }
  end

  def test_add_instance
    definition = @model.definitions['Heather']
    instance = @entities.add_instance(definition, Geom::Transformation.translation([100, 0, 0]))
    assert_instance_of(Sketchup::ComponentInstance, instance)
    assert(@entities.include?(instance))
    assert_instance_of(Sketchup::ComponentInstance, @entities.add_instance(definition, [5, 5, 5]))
    assert_raises(TypeError) { @entities.add_instance('x', Geom::Transformation.new) }
  end

  def test_add_image
    image = @entities.add_image(File.join(TEST_RESOURCES, 'texture_map.png'), [0, 0, 0], 100)
    assert_instance_of(Sketchup::Image, image)
    assert(@entities.include?(image))
    assert_nil(@entities.add_image('/does/not/exist.png', [0, 0, 0], 100))
  end

  def test_add_section_plane
    plane = @entities.add_section_plane([0, 0, 5], [0, 0, 1])
    assert_instance_of(Sketchup::SectionPlane, plane)
    assert_instance_of(Sketchup::SectionPlane, @entities.add_section_plane([[0, 0, 1], [0, 0, 1]]))
    assert_instance_of(Sketchup::SectionPlane, @entities.add_section_plane([0, 0, 1, -3]))
  end

  def test_active_section_plane
    assert_nil(@entities.active_section_plane)
    plane = @entities.add_section_plane([0, 0, 5], [0, 0, 1])
    assert_equal(plane, @entities.active_section_plane = plane)
    assert_equal(plane, @entities.active_section_plane)
    assert_nil(@entities.active_section_plane = nil)
    assert_nil(@entities.active_section_plane)
  end

  def test_add_text
    text = @entities.add_text('Hello', [0, 0, 0], [1, 1, 1])
    assert_instance_of(Sketchup::Text, text)
    assert(@entities.include?(text))
  end

  def test_add_faces_from_mesh
    mesh = Geom::PolygonMesh.new
    mesh.add_polygon([0, 0, 0], [10, 0, 0], [10, 10, 0])
    mesh.add_polygon([0, 0, 0], [10, 10, 0], [0, 10, 0])
    target = @model.definitions.add('Mesh').entities
    assert_equal(2, target.add_faces_from_mesh(mesh))
    assert_raises(TypeError) { @entities.add_faces_from_mesh('x') }
  end

  def test_fill_from_mesh
    mesh = Geom::PolygonMesh.new
    mesh.add_polygon([0, 0, 0], [10, 0, 0], [10, 10, 0])
    target = @model.definitions.add('Mesh').entities
    assert(target.fill_from_mesh(mesh, true, Geom::PolygonMesh::NO_SMOOTH_OR_HIDE))
    assert_equal(1, target.grep(Sketchup::Face).size)
  end

  def test_erase_entities
    edge = @entities.add_line([0, 0, 0], [1, 0, 0])
    count = @entities.count
    @entities.erase_entities(edge)
    assert_equal(count - 1, @entities.count)
    refute(@entities.include?(edge))
    assert_raises(TypeError) { @entities.erase_entities('x') }
  end

  def test_clear!
    target = @model.definitions.add('Clear').entities
    target.add_line([0, 0, 0], [1, 0, 0])
    assert(target.clear!)
    assert_equal(0, target.count)
  end

  def test_transform_entities
    point = @entities.add_cpoint([0, 0, 0])
    assert(@entities.transform_entities(Geom::Transformation.translation([5, 0, 0]), [point]))
    assert(@entities.transform_entities(Geom::Transformation.translation([5, 0, 0]), point))
  end

  def test_transform_by_vectors
    a = @entities.add_cpoint([0, 0, 0])
    b = @entities.add_cpoint([1, 0, 0])
    assert_same(@entities, @entities.transform_by_vectors([a, b], [[1, 0, 0], [0, 1, 0]]))
    assert_raises(ArgumentError) { @entities.transform_by_vectors([a, b], [[1, 0, 0]]) }
  end

  def test_weld
    edges = @entities.add_edges([0, 0, 0], [1, 0, 0], [2, 1, 0], [3, 0, 0])
    curves = @entities.weld(edges)
    assert_kind_of(Array, curves)
    curves.each { |c| assert_kind_of(Sketchup::Curve, c) }
  end

  def test_equality
    assert(@model.entities == @model.entities)
  end
end
