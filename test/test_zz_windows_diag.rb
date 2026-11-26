require_relative 'helper'

# Temporary: prints how scenes behave on CI.
class TestZzWindowsDiag < Minitest::Test
  def test_diag
    model = open_model
    pages = model.pages
    a = pages.add('Plan')
    puts "DIAG add=#{a.name.inspect} id=#{a.entityID} list=#{pages.to_a.map { |p| [p.name, p.entityID] }.inspect}"
    puts "DIAG byname=#{pages['Plan']&.entityID.inspect} unique=#{pages.unique_name('Plan').inspect}"
    b = pages.add('First', PAGE_USE_ALL, 0) rescue puts("DIAG add-index0 #{$!.message}")
    c = pages.add('Second') rescue puts("DIAG add-second #{$!.message}")
    puts "DIAG after=#{pages.to_a.map { |p| [p.name, p.entityID] }.inspect} b=#{b&.name.inspect} c=#{c&.name.inspect}"
    assert(true)
  end
end
