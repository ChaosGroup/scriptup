# About

This project provides ruby bindings for the SketchUp C API. They are designed to mimic the SketchUp [**ruby api**](https://ruby.sketchup.com) for use in SketchUp extension tests outside the SketchUp application. You can however use them for general purpose scripting.

The SketchUp C API is much more limited than the ruby api. There is no gui event loop, nor observers, nor view, nor does this extension aim to provide such functionality.

# Implementation status

Method counts are against the SketchUp 2026.0 ruby api.

## Fully implemented
| Class | Methods |
|---|---|
| Sketchup::ArcCurve | 9/9 |
| Sketchup::Axes | 9/9 |
| Sketchup::Behavior | 12/12 |
| Sketchup::Camera | 27/27 |
| Sketchup::Color | 15/15 |
| Sketchup::ConstructionPoint | 1/1 |
| Sketchup::Curve | 9/9 |
| Sketchup::Drawingelement | 14/14 |
| Sketchup::EdgeUse | 9/9 |
| Sketchup::EntitiesBuilder | 6/6 (geometry is added immediately, not merged) |
| Sketchup::ImageRep | 12/12 |
| Sketchup::InstancePath | 14/14 |
| Sketchup::Layer | 15/15 |
| Sketchup::LineStyle | 1/1 |
| Sketchup::LineStyles | 6/6 |
| Sketchup::Loop | 6/6 |
| Sketchup::SectionPlane | 8/8 |
| Sketchup::Texture | 10/10 |
| Sketchup::TextureWriter | 7/7 |
| Sketchup::UVHelper | 2/2 |
| Sketchup::Vertex | 7/7 |
| Geom (module) | 7/8 (no `tesselate`: no C API) |
| Geom::BoundingBox | 15/15 |
| Geom::LatLong | 5/6 (no `to_utm`: no C API) |
| Geom::Point2d | 21/21 |
| Geom::Point3d | 31/31 |
| Geom::PolygonMesh | 17/17 |
| Geom::Transformation | 17/17 |
| Geom::Transformation2d | 12/12 |
| Geom::Vector2d | 33/33 |
| Geom::Vector3d | 38/38 |
| Length (plus the Numeric and String#to_l extensions) | 9/9 |

## Mostly implemented
| Class | Methods | Missing / incomplete |
|---|---|---|
| Sketchup::Image | 16/19 | no `explode` or `glued_to` (no C API for images) |
| Sketchup::LayerFolder | 18/20 | no `visible_on_new_pages` (no C API); `folder = nil` can't move a folder to the top level |
| Sketchup::Material | 45/46 | no `write_thumbnail` (no C API) |
| Sketchup::Materials | 10/15 | no `current`, `purge_unused` or observers (no C API) |
| Sketchup::Text | 16/18 | no `line_weight` (no C API) |
| Sketchup::Layers | 19/21 | no observers (no C API); `remove` can't remove a tag's geometry |
| Sketchup::Entity | 14/16 | no observers (no C API); `delete_attribute` can't delete a single key |
| Sketchup::AttributeDictionary | 12/13 | `empty?`; `delete_key` doesn't delete |
| Sketchup::AttributeDictionaries | 5/6 | `delete` |

## Partially implemented
| Class | Methods |
|---|---|
| Sketchup::ComponentDefinition | 22/34 (`add_classification` with Classifications; no thumbnails, `insertion_point=`, `invalidate_bounds`, classification values or observers: no C API) |
| Sketchup::DefinitionList | 12/15 (no `load_from_url` or observers: no C API; `import` handles .skp only) |
| Sketchup::Model | 9/73 |
| Sketchup::Edge | 16/19 (no `explode_curve`, `find_faces`, `split`: no C API) |
| Sketchup::Face | 25/28 (no `pushpull`, `followme`, `set_texture_projection`: no C API) |
| Sketchup::ComponentInstance | 13/27 (no `definition=`, booleans, `explode`, `make_unique`, `glued_to=` or observers: no C API) |
| Sketchup::ConstructionLine | 4/11 (read-only: no setters, `reverse!` or stipple in the C API) |
| Sketchup::Group | 17/32 (no booleans, `copy`, `explode`, `make_unique`, `to_component`, `glued_to=` or observers: no C API) |
| Sketchup::Entities | 32/39 (no 3D text, intersect, dimensions, snaps or observers in the C API) |
| Sketchup (module) | 2/61 (`open_file`, `active_model`) |

## Defined, no methods of their own yet
Sketchup::Dimension

# Supportable through the C API

These ruby api classes have a C API counterpart, so they can be implemented (at least partially):

* **Geom:** BoundingBox, LatLong, Point2d, Point3d, PolygonMesh, Transformation, Transformation2d, Vector2d, Vector3d
* **Model and collections:** Model, Entities, EntitiesBuilder, DefinitionList, Materials, Layers, Pages, Styles, Environments, LineStyles, Selection, Classifications, OptionsManager, OptionsProvider, RenderingOptions, ShadowInfo
* **Entities:** Entity, Drawingelement, ArcCurve, Axes, ComponentDefinition, ComponentInstance, ConstructionLine, ConstructionPoint, Curve, Dimension, DimensionLinear, DimensionRadial, Edge, EdgeUse, Environment, Face, Group, Image, Layer, LayerFolder, Loop, Material, Page, SectionPlane, Text, Texture, Vertex
* **Other:** AttributeDictionary, AttributeDictionaries, Behavior, Camera, ClassificationSchema, Color, ImageRep, InstancePath, LineStyle, Snap, Style, TextureWriter, UVHelper, Licensing::ExtensionLicense, Length

Not supportable (no C API): UI, View, Tool/Tools, InputPoint, PickHelper, Animation, observers, Console, Menu, Http, ExtensionsManager, Overlay/OverlaysManager, Importer, and the LayOut api.

# Type signatures
`bundle exec rake rbs` generates RBS for the SketchUp 2026.0 ruby api into `sig/` from [sketchup-api-stubs](https://github.com/SketchUp/sketchup-api-stubs); the gem ships them. See `scripts/rbs/`.
