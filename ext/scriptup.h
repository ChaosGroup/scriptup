#ifndef SCRIPTUP_H
#define SCRIPTUP_H

#include <ruby.h>
#include <stdbool.h>
#include <SketchUpAPI/sketchup.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// X(var, namespace, "Name", parent) in definition order (parents first).
#define SCRIPTUP_CLASSES(X) \
	X(cLength, rb_cObject, "Length", rb_cNumeric) \
	X(cModel, mSketchup, "Model", rb_cObject) \
	X(cColor, mSketchup, "Color", rb_cObject) \
	X(cImageRep, mSketchup, "ImageRep", rb_cObject) \
	X(cEntities, mSketchup, "Entities", rb_cObject) \
	X(cBoundingBox, mGeom, "BoundingBox", rb_cObject) \
	X(cVector3d, mGeom, "Vector3d", rb_cObject) \
	X(cPoint3d, mGeom, "Point3d", rb_cObject) \
	X(cVector2d, mGeom, "Vector2d", rb_cObject) \
	X(cPoint2d, mGeom, "Point2d", rb_cObject) \
	X(cTransformation, mGeom, "Transformation", rb_cObject) \
	X(cTransformation2d, mGeom, "Transformation2d", rb_cObject) \
	X(cLatLong, mGeom, "LatLong", rb_cObject) \
	X(cPolygonMesh, mGeom, "PolygonMesh", rb_cObject) \
	X(cInstancePath, mSketchup, "InstancePath", rb_cObject) \
	X(cEntitiesBuilder, mSketchup, "EntitiesBuilder", rb_cObject) \
	X(cTextureWriter, mSketchup, "TextureWriter", rb_cObject) \
	X(cUVHelper, mSketchup, "UVHelper", rb_cObject) \
	X(cCamera, mSketchup, "Camera", rb_cObject) \
	X(cOptionsManager, mSketchup, "OptionsManager", rb_cObject) \
	X(cOptionsProvider, mSketchup, "OptionsProvider", rb_cObject) \
	X(cClassifications, mSketchup, "Classifications", rb_cObject) \
	X(cClassificationSchema, mSketchup, "ClassificationSchema", rb_cObject) \
	X(cEntity, mSketchup, "Entity", rb_cObject) \
	X(cMaterials, mSketchup, "Materials", cEntity) \
	X(cMaterial, mSketchup, "Material", cEntity) \
	X(cDefinitionList, mSketchup, "DefinitionList", cEntity) \
	X(cAttributeDictionary, mSketchup, "AttributeDictionary", cEntity) \
	X(cAttributeDictionaries, mSketchup, "AttributeDictionaries", cEntity) \
	X(cTexture, mSketchup, "Texture", cEntity) \
	X(cBehavior, mSketchup, "Behavior", cEntity) \
	X(cAxes, mSketchup, "Axes", cEntity) \
	X(cPage, mSketchup, "Page", cEntity) \
	X(cPages, mSketchup, "Pages", cEntity) \
	X(cRenderingOptions, mSketchup, "RenderingOptions", cEntity) \
	X(cShadowInfo, mSketchup, "ShadowInfo", cEntity) \
	X(cStyle, mSketchup, "Style", cEntity) \
	X(cStyles, mSketchup, "Styles", cEntity) \
	X(cEnvironment, mSketchup, "Environment", cEntity) \
	X(cEnvironments, mSketchup, "Environments", cEntity) \
	X(cLayer, mSketchup, "Layer", cEntity) \
	X(cLayers, mSketchup, "Layers", cEntity) \
	X(cLayerFolder, mSketchup, "LayerFolder", cEntity) \
	X(cLineStyle, mSketchup, "LineStyle", cEntity) \
	X(cLineStyles, mSketchup, "LineStyles", cEntity) \
	X(cVertex, mSketchup, "Vertex", cEntity) \
	X(cLoop, mSketchup, "Loop", cEntity) \
	X(cEdgeUse, mSketchup, "EdgeUse", cEntity) \
	X(cCurve, mSketchup, "Curve", cEntity) \
	X(cArcCurve, mSketchup, "ArcCurve", cCurve) \
	X(cDrawingelement, mSketchup, "Drawingelement", cEntity) \
	X(cComponentDefinition, mSketchup, "ComponentDefinition", cDrawingelement) \
	X(cComponentInstance, mSketchup, "ComponentInstance", cDrawingelement) \
	X(cConstructionLine, mSketchup, "ConstructionLine", cDrawingelement) \
	X(cConstructionPoint, mSketchup, "ConstructionPoint", cDrawingelement) \
	X(cDimension, mSketchup, "Dimension", cDrawingelement) \
	X(cEdge, mSketchup, "Edge", cDrawingelement) \
	X(cFace, mSketchup, "Face", cDrawingelement) \
	X(cGroup, mSketchup, "Group", cDrawingelement) \
	X(cImage, mSketchup, "Image", cDrawingelement) \
	X(cSectionPlane, mSketchup, "SectionPlane", cDrawingelement) \
	X(cText, mSketchup, "Text", cDrawingelement)

#define SCRIPTUP_DECLARE_CLASS(var, ns, name, parent) extern VALUE var;
extern VALUE mSketchup;
extern VALUE mGeom;
SCRIPTUP_CLASSES(SCRIPTUP_DECLARE_CLASS)

// Per-file method definitions, called after all classes exist.
void Init_arc_curve(void);
void Init_attribute_dictionaries(void);
void Init_axes(void);
void Init_attribute_dictionary(void);
void Init_behavior(void);
void Init_bounding_box(void);
void Init_camera(void);
void Init_classifications(void);
void Init_color(void);
void Init_component_definition(void);
void Init_component_instance(void);
void Init_construction(void);
void Init_curve(void);
void Init_definition_list(void);
void Init_drawing_element(void);
void Init_entities(void);
void Init_entities_builder(void);
void Init_edge(void);
void Init_edge_use(void);
void Init_entity(void);
void Init_environments(void);
void Init_face(void);
void Init_image(void);
void Init_image_rep(void);
void Init_instance_path(void);
void Init_lat_long(void);
void Init_geom(void);
void Init_group(void);
void Init_layer(void);
void Init_layer_folder(void);
void Init_layers(void);
void Init_length(void);
void Init_line_styles(void);
void Init_loop(void);
void Init_material(void);
void Init_materials(void);
void Init_model(void);
void Init_options_store(void);
void Init_page(void);
void Init_pages(void);
void Init_point2d(void);
void Init_polygon_mesh(void);
void Init_point3d(void);
void Init_section_plane(void);
void Init_styles(void);
void Init_text(void);
void Init_texture(void);
void Init_texture_writer(void);
void Init_transformation(void);
void Init_transformation2d(void);
void Init_uv_helper(void);
void Init_vector2d(void);
void Init_vector3d(void);
void Init_vertex(void);

SUModelRef Sketchup_get_active_model(void);
void Sketchup_set_active_model(SUModelRef model);

// helpers.c
VALUE su_wrap(VALUE klass, void* ptr);
VALUE su_wrap_or_nil(VALUE klass, void* ptr);
VALUE su_wrap_entity(SUEntityRef entity);
SUMaterialRef su_material_from_arg(VALUE entity, VALUE material);
SUInstancePathRef su_to_instance_path(VALUE obj);
VALUE su_instance_path_copy(SUInstancePathRef source);
VALUE su_image_rep_new(SUImageRepRef image);
VALUE su_camera_wrap(SUCameraRef camera);
VALUE su_instance_locked_p(VALUE self);
VALUE su_instance_set_locked(VALUE self, VALUE lock);
VALUE su_instance_volume(VALUE self);
VALUE su_instance_manifold_p(VALUE self);
VALUE su_instance_glued_to(VALUE self);
void* su_unwrap(VALUE obj, VALUE klass);
VALUE su_string_to_rb(SUStringRef string);
int su_clamp_int(VALUE value, int min, int max);
double su_clamp_double(VALUE value, double min, double max);
VALUE su_yield_each(VALUE self, VALUE ary);
VALUE su_find_by(VALUE ary, const char* method, VALUE key);
VALUE su_fetch(VALUE ary, VALUE index);
VALUE su_length_new(double inches);
VALUE su_model_attribute_dictionaries(SUModelRef model);
VALUE su_color_new(SUColor value);
VALUE su_color_coerce(VALUE obj);
VALUE su_point3d_new(struct SUPoint3D value);
struct SUPoint3D su_to_point3d(VALUE obj);
struct SUPoint3D su_to_point3d_like(VALUE obj);
VALUE su_entities_new(SUEntitiesRef entities);
struct SURay3D su_to_ray3d(VALUE line);
struct SUPlane3D su_to_plane3d(VALUE plane);
VALUE su_vector3d_new(struct SUVector3D value);
struct SUVector3D su_to_vector3d(VALUE obj);
void su_array_to_xyz(VALUE ary, double xyz[3]);
void su_array_to_xy(VALUE ary, double xy[2]);
VALUE su_polygon_mesh_new(VALUE points, VALUE polygons, VALUE uvs_front, VALUE uvs_back, VALUE normals);
VALUE su_lat_long_new(double latitude, double longitude);
VALUE su_bounding_box_new(struct SUBoundingBox3D value);
VALUE su_transformation_new(struct SUTransformation value);
struct SUTransformation su_to_transformation(VALUE obj);
struct SUTransformation2D su_to_transformation2d(VALUE obj);
VALUE su_point2d_new(struct SUPoint2D value);
struct SUPoint2D su_to_point2d(VALUE obj);
VALUE su_vector2d_new(struct SUVector2D value);
struct SUVector2D su_to_vector2d(VALUE obj);
VALUE su_format_number(double value);
VALUE su_ruby_from_typed_value(SUTypedValueRef value);
void su_typed_value_from_ruby(SUTypedValueRef value, VALUE object);

#define BOOL2RB(b) ((b) ? Qtrue : Qfalse)
#define SU_REF(Type, obj) ((Type){DATA_PTR(obj)})

// Method generators. Function names follow <class>_<method>, with set_ for
// writers and _p for predicates, so they can be generated from the RBS.
#define DEFINE_GETTER(fn, Type, SUFn, CType, init, TO_RUBY) \
	static VALUE fn(VALUE self) \
	{ \
		CType value = init; \
		SUFn(SU_REF(Type, self), &value); \
		return TO_RUBY(value); \
	}

#define DEFINE_SETTER(fn, Type, SUFn, FROM_RUBY) \
	static VALUE fn(VALUE self, VALUE value) \
	{ \
		SUFn(SU_REF(Type, self), FROM_RUBY(value)); \
		return value; \
	}

#define DEFINE_BOOL_GETTER(fn, Type, SUFn) DEFINE_GETTER(fn, Type, SUFn, bool, false, BOOL2RB)
#define DEFINE_BOOL_SETTER(fn, Type, SUFn) DEFINE_SETTER(fn, Type, SUFn, RTEST)
#define DEFINE_SIZE_GETTER(fn, Type, SUFn) DEFINE_GETTER(fn, Type, SUFn, size_t, 0, SIZET2NUM)
#define DEFINE_DOUBLE_GETTER(fn, Type, SUFn) DEFINE_GETTER(fn, Type, SUFn, double, 0.0, DBL2NUM)
#define DEFINE_ENUM_GETTER(fn, Type, SUFn, EnumType) DEFINE_GETTER(fn, Type, SUFn, EnumType, 0, INT2NUM)
#define DEFINE_STRING_SETTER(fn, Type, SUFn) DEFINE_SETTER(fn, Type, SUFn, StringValueCStr)

#define DEFINE_STRING_GETTER(fn, Type, SUFn) \
	static VALUE fn(VALUE self) \
	{ \
		SUStringRef string = SU_INVALID; \
		SUStringCreate(&string); \
		SUFn(SU_REF(Type, self), &string); \
		VALUE output = su_string_to_rb(string); \
		SUStringRelease(&string); \
		return output; \
	}

#define DEFINE_REF_GETTER(fn, Type, SUFn, OutType, klass) \
	static VALUE fn(VALUE self) \
	{ \
		OutType out = SU_INVALID; \
		SUFn(SU_REF(Type, self), &out); \
		return su_wrap_or_nil(klass, out.ptr); \
	}

// static VALUE fn(Type ref): Ruby array of wrapped refs from a Num/Get pair.
#define DEFINE_REF_LIST(fn, Type, NumFn, GetFn, ItemType, klass) \
	static VALUE fn(Type ref) \
	{ \
		size_t count = 0; \
		NumFn(ref, &count); \
		VALUE ary = rb_ary_new_capa((long)count); \
		if (count == 0) \
			return ary; \
		ItemType* items = ALLOC_N(ItemType, count); \
		GetFn(ref, count, items, &count); \
		for (size_t i = 0; i < count; i++) \
			rb_ary_push(ary, su_wrap(klass, items[i].ptr)); \
		xfree(items); \
		return ary; \
	}

// static VALUE fn(Type ref): Ruby array of strings from a Num/Get pair.
#define DEFINE_STRING_LIST(fn, Type, NumFn, GetFn) \
	static VALUE fn(Type ref) \
	{ \
		size_t count = 0; \
		NumFn(ref, &count); \
		VALUE ary = rb_ary_new_capa((long)count); \
		if (count == 0) \
			return ary; \
		SUStringRef* strings = ALLOC_N(SUStringRef, count); \
		for (size_t i = 0; i < count; i++) \
		{ \
			strings[i].ptr = NULL; \
			SUStringCreate(&strings[i]); \
		} \
		GetFn(ref, count, strings, &count); \
		for (size_t i = 0; i < count; i++) \
		{ \
			rb_ary_push(ary, su_string_to_rb(strings[i])); \
			SUStringRelease(&strings[i]); \
		} \
		xfree(strings); \
		return ary; \
	}

#endif // SCRIPTUP_H
