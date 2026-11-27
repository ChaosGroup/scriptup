require_relative 'helper'

class TestEnvironments < Minitest::Test
  # No C API: the source path, choosing the current environment, purging and observers.
  UNSUPPORTED = %i[current= purge_unused add_observer remove_observer].freeze
  ENVIRONMENT_UNSUPPORTED = %i[path].freeze

  def setup
    @model = open_model
    @environments = @model.environments
    @dir = Dir.mktmpdir
    @hdr = write_hdr(File.join(@dir, 'sky.hdr'))
  end

  def teardown
    FileUtils.remove_entry(@dir)
  end

  def test_unsupported
    assert_unsupported(@environments, UNSUPPORTED)
    assert_unsupported(@environments.add('Sky', @hdr), ENVIRONMENT_UNSUPPORTED)
  end

  def test_collection
    assert_instance_of(Sketchup::Environments, @environments)
    assert_kind_of(Enumerable, @environments)
    before = @environments.size
    environment = @environments.add('Sky', @hdr)
    assert_instance_of(Sketchup::Environment, environment)
    assert_equal(before + 1, @environments.size)
    assert_equal(environment, @environments['Sky'])
    assert_nil(@environments['Nope'])
    assert_same(@environments, @environments.each { |e| assert_instance_of(Sketchup::Environment, e) })
    assert_instance_of(Sketchup::Environment, @environments.add(@hdr))
    assert_raises(ArgumentError) { @environments.add('/does/not/exist.hdr') }
    assert(@environments.remove(environment))
    assert_nil(@environments['Sky'])
    assert_nil(@environments.current)
  end

  def test_environment
    environment = @environments.add('Sky', @hdr)
    assert_equal('Sky', environment.name)
    assert_equal('Clouds', environment.name = 'Clouds')
    assert_equal('About', environment.description = 'About')
    assert_equal('About', environment.description)
    %i[use_as_skydome use_for_reflections linked_sun].each do |flag|
      environment.public_send("#{flag}=", true)
      assert(environment.public_send("#{flag}?"), flag)
      environment.public_send("#{flag}=", false)
      refute(environment.public_send("#{flag}?"), flag)
    end
    environment.rotation = 45.0
    assert_in_delta(45.0, environment.rotation)
    environment.skydome_exposure = 1.5
    assert_in_delta(1.5, environment.skydome_exposure)
    environment.reflection_exposure = 0.5
    assert_in_delta(0.5, environment.reflection_exposure)
  end

  def test_linked_sun_position
    environment = @environments.add('Sky', @hdr)
    environment.linked_sun_position = Geom::Point3d.new(0.25, 0.75, 0)
    position = environment.linked_sun_position
    assert_instance_of(Geom::Point3d, position)
    assert_in_delta(0.25, position.x.to_f)
    assert_in_delta(0.75, position.y.to_f)
  end

  def test_thumbnail_and_write_hdr
    environment = @environments.add('Sky', @hdr)
    assert_instance_of(Sketchup::ImageRep, environment.thumbnail)
    out = Dir.mktmpdir
    path = environment.write_hdr(out)
    assert(File.exist?(path))
    FileUtils.remove_entry(out)
  end

  def test_page_environment
    environment = @environments.add('Sky', @hdr)
    page = @model.pages.add('Scene')
    assert_equal(environment, page.environment = environment)
    assert_equal(environment, page.environment)
  end
end
