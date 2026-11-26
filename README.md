# scriptup

Ruby bindings for the SketchUp C API that mimic the SketchUp [Ruby API](https://ruby.sketchup.com), so extension tests can run outside SketchUp. macOS and Windows.

## Status (SketchUp 2026.0 API)

Everything the standalone C API can support is implemented and tested: the Geom classes, Length, entities and geometry, components and groups, tags, materials and textures, images, annotations, scenes, styles, cameras, rendering/shadow options, classifications and environments.

Methods that have no C API counterpart are left undefined. Each test file lists them in `UNSUPPORTED`. Run `bundle exec rake rbs:coverage` for per-class counts and missing methods.

## Gaps

- **No C API:** observers, UI/View/Tools, operations (undo), export/import, boolean ops, pushpull/followme, explode, and app-level `Sketchup` module functions.
- **Only inside SketchUp:** Selection, Licensing, Sketchup version/edition.
- **Blocked on fixtures:** Dimension, DimensionLinear, DimensionRadial and Snap need a test model that contains them; the C API can't create them in a model.
- **Partial:** geometry added through the API isn't merged as in SketchUp; construction lines are read-only.

## Direction

The bindings follow RBS generated from the Ruby API docs (`scripts/rbs/`, shipped in `sig/`). Next, generate much of the C binding code from that RBS using the `DEFINE_*` macros in `ext/scriptup.h`.

## Development

```sh
bundle exec rake               # download SDK, compile, test
bundle exec rake rbs           # generate and validate RBS
bundle exec rake rbs:coverage  # implemented vs. Ruby API, per class
bundle exec rake check_symbols # C API calls the SDK library doesn't export
```
