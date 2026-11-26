require_relative 'helper'

class TestConstructionLine < Minitest::Test
  # No C API to edit construction lines or read/set their stipple.
  UNSUPPORTED = %i[direction= end= position= start= reverse! stipple stipple=].freeze

  def setup
    @model = open_model
    @finite = @model.entities.add_cline([1, 0, 0], [5, 0, 0])
    @infinite = @model.entities.add_cline([0, 2, 0], Geom::Vector3d.new(0, 3, 0))
  end

  def test_unsupported
    assert_unsupported(@finite, UNSUPPORTED)
  end

  def test_position
    assert_equal(Geom::Point3d.new(1, 0, 0), @finite.position)
    assert_equal(Geom::Point3d.new(0, 2, 0), @infinite.position)
  end

  def test_direction
    assert(@finite.direction.samedirection?([1, 0, 0]))
    assert(@finite.direction.unitvector?)
    assert(@infinite.direction.samedirection?([0, 1, 0]))
  end

  def test_start_and_end
    assert_equal(Geom::Point3d.new(1, 0, 0), @finite.start)
    assert_equal(Geom::Point3d.new(5, 0, 0), @finite.end)
    assert_nil(@infinite.start)
    assert_nil(@infinite.end)
  end
end
