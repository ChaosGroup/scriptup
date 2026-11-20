#ifndef SCRIPTUP_H
#define SCRIPTUP_H

#include <ruby.h>
#include <stdbool.h>
#include <SketchUpAPI/sketchup.h>

// X(var, namespace, "Name", parent) in definition order (parents first).
#define SCRIPTUP_CLASSES(X) \
	X(cModel, mSketchup, "Model", rb_cObject) \
	X(cColor, mSketchup, "Color", rb_cObject) \
	X(cImageRep, mSketchup, "ImageRep", rb_cObject) \
	X(cEntities, mSketchup, "Entities", rb_cObject) \
	X(cBoundingBox, mGeom, "BoundingBox", rb_cObject) \
	X(cEntity, mSketchup, "Entity", rb_cObject) \
	X(cMaterials, mSketchup, "Materials", cEntity) \
	X(cMaterial, mSketchup, "Material", cEntity) \
	X(cDefinitionList, mSketchup, "DefinitionList", cEntity) \
	X(cAttributeDictionary, mSketchup, "AttributeDictionary", cEntity) \
	X(cAttributeDictionaries, mSketchup, "AttributeDictionaries", cEntity) \
	X(cTexture, mSketchup, "Texture", cEntity) \
	X(cBehavior, mSketchup, "Behavior", cEntity) \
	X(cLayer, mSketchup, "Layer", cEntity) \
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
void Init_attribute_dictionaries(void);
void Init_attribute_dictionary(void);
void Init_color(void);
void Init_component_definition(void);
void Init_definition_list(void);
void Init_drawing_element(void);
void Init_entities(void);
void Init_entity(void);
void Init_material(void);
void Init_materials(void);
void Init_model(void);
void Init_texture(void);

SUModelRef Sketchup_get_active_model(void);
void Sketchup_set_active_model(SUModelRef model);

// helpers.c
VALUE su_wrap(VALUE klass, void* ptr);
VALUE su_wrap_or_nil(VALUE klass, void* ptr);
void* su_unwrap(VALUE obj, VALUE klass);
VALUE su_string_to_rb(SUStringRef string);
int su_clamp_int(VALUE value, int min, int max);
double su_clamp_double(VALUE value, double min, double max);
VALUE su_yield_each(VALUE self, VALUE ary);
VALUE su_find_by(VALUE ary, const char* method, VALUE key);
VALUE su_fetch(VALUE ary, VALUE index);
VALUE su_model_attribute_dictionaries(SUModelRef model);
VALUE su_color_new(SUColor value);
VALUE su_color_coerce(VALUE obj);
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
