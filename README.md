# About

This project provides ruby bindings for the SketchUp C API. They are designed to mimic the SketchUp [**ruby api**](https://ruby.sketchup.com) for use in SketchUp extension tests outside the SketchUp application. You can however use them for general purpose scripting.

The SketchUp C API is much more limited than the ruby api. There is no gui event loop, nor observers, nor view, nor does this extension aim to provide such functionality.

# Implementation status

Method counts are against the SketchUp 2026.0 ruby api.

## Fully implemented
| Class | Methods |
|---|---|
| Sketchup::Color | 15/15 |
| Geom::BoundingBox | 15/15 |
| Geom::LatLong | 5/6 (no `to_utm`: no C API) |
| Geom::Point2d | 21/21 |
| Geom::Point3d | 31/31 |
| Geom::Transformation | 17/17 |
| Geom::Transformation2d | 12/12 |
| Geom::Vector2d | 33/33 |
| Geom::Vector3d | 38/38 |
| Length (plus the Numeric and String#to_l extensions) | 9/9 |

## Mostly implemented
| Class | Methods | Missing / incomplete |
|---|---|---|
| Sketchup::Entity | 16/16 | `get_attribute` and `set_attribute` are stubs; `delete_attribute` ignores a key |
| Sketchup::Drawingelement | 13/14 | `erase!` |
| Sketchup::AttributeDictionary | 12/13 | `empty?`; `delete_key` doesn't delete |
| Sketchup::Texture | 9/10 | `size=` |
| Sketchup::AttributeDictionaries | 5/6 | `delete` |

## Partially implemented
| Class | Methods |
|---|---|
| Sketchup::ComponentDefinition | 21/34 |
| Sketchup::DefinitionList | 10/15 |
| Sketchup::Material | 17/46 (no PBR properties) |
| Sketchup::Materials | 5/15 |
| Sketchup::Model | 4/73 |
| Sketchup::Entities | 2/39 (`each` yields faces only) |
| Sketchup (module) | 2/61 (`open_file`, `active_model`) |

## Defined, no methods of their own yet
Sketchup::Behavior, Sketchup::ComponentInstance, Sketchup::ConstructionLine, Sketchup::ConstructionPoint, Sketchup::Dimension, Sketchup::Edge, Sketchup::Face, Sketchup::Group, Sketchup::Image, Sketchup::ImageRep, Sketchup::Layer, Sketchup::SectionPlane, Sketchup::Text

# Supportable through the C API

These ruby api classes have a C API counterpart, so they can be implemented (at least partially):

* **Geom:** BoundingBox, LatLong, Point2d, Point3d, PolygonMesh, Transformation, Transformation2d, Vector2d, Vector3d
* **Model and collections:** Model, Entities, EntitiesBuilder, DefinitionList, Materials, Layers, Pages, Styles, Environments, LineStyles, Selection, Classifications, OptionsManager, OptionsProvider, RenderingOptions, ShadowInfo
* **Entities:** Entity, Drawingelement, ArcCurve, Axes, ComponentDefinition, ComponentInstance, ConstructionLine, ConstructionPoint, Curve, Dimension, DimensionLinear, DimensionRadial, Edge, EdgeUse, Environment, Face, Group, Image, Layer, LayerFolder, Loop, Material, Page, SectionPlane, Text, Texture, Vertex
* **Other:** AttributeDictionary, AttributeDictionaries, Behavior, Camera, ClassificationSchema, Color, ImageRep, InstancePath, LineStyle, Snap, Style, TextureWriter, UVHelper, Licensing::ExtensionLicense, Length

Not supportable (no C API): UI, View, Tool/Tools, InputPoint, PickHelper, Animation, observers, Console, Menu, Http, ExtensionsManager, Overlay/OverlaysManager, Importer, and the LayOut api.

# Type signatures
`bundle exec rake rbs` generates RBS for the SketchUp 2026.0 ruby api into `sig/` from [sketchup-api-stubs](https://github.com/SketchUp/sketchup-api-stubs); the gem ships them. See `scripts/rbs/`.
