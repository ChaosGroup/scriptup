# Get handle on the active model
Sketchup.active_model

# Run all tests
Dir.glob('test/**/test_*.rb') { |p|
  require_relative p
}
