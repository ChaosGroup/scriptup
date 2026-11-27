require_relative 'helper'

class TestTextureWriter < Minitest::Test
  def setup
    @model = open_model
    @material = @model.materials['Heather_Shirt']
    @material.texture = File.join(TEST_RESOURCES, 'texture_map.png')
    @face = add_square_face(@model.entities)
    @face.material = @material
    @writer = Sketchup.create_texture_writer
  end

  def test_create
    assert_instance_of(Sketchup::TextureWriter, @writer)
    assert_raises(NoMethodError) { Sketchup::TextureWriter.new }
  end

  def test_constants
    assert_equal([0, 1, 2], [FILE_WRITE_OK, FILE_WRITE_FAILED_INVALID_TYPE, FILE_WRITE_FAILED_UNKNOWN])
  end

  def test_load_and_count
    assert_equal(0, @writer.count)
    handle = @writer.load(@face, true)
    assert_instance_of(Integer, handle)
    assert_operator(handle, :>, 0)
    assert_equal(1, @writer.count)
    assert_equal(1, @writer.length)
    assert_equal(handle, @writer.handle(@face, true))
  end

  def test_filename
    handle = @writer.load(@face, true)
    assert_raises(ArgumentError, 'only known after write_all') { @writer.filename(handle) }
    Dir.mktmpdir do |dir|
      @writer.write_all(dir, false)
      assert_equal("texture_map.png", @writer.filename(handle))
      assert(File.exist?(File.join(dir, @writer.filename(handle))))
    end
    assert_raises(ArgumentError) { @writer.filename(999) }
  end

  def test_write
    @writer.load(@face, true)
    Dir.mktmpdir do |dir|
      path = File.join(dir, 'front.png')
      assert_equal(FILE_WRITE_OK, @writer.write(@face, true, path))
      assert(File.exist?(path))
      assert_equal(FILE_WRITE_FAILED_UNKNOWN, @writer.write(add_square_face(@model.entities, 5, 9), File.join(dir, 'x.png')))
    end
  end

  def test_write_all
    @writer.load(@face, true)
    Dir.mktmpdir do |dir|
      assert_equal(FILE_WRITE_OK, @writer.write_all(dir, false))
      refute_empty(Dir.children(dir))
    end
  end

  def test_type_errors
    assert_raises(TypeError) { @writer.load('x') }
    assert_raises(TypeError) { @writer.handle(1) }
  end
end
