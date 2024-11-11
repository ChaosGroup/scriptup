#include <stdbool.h>
#include <materials.h>
#include <SketchUpAPI/sketchup.h>
#include <utils.h>

void Sketchup_Materials_Iterator(SUMaterialRef material, void* _)
{
	rb_yield(Data_Wrap_Struct(rb_path2class(SKETCHUP_MATERIAL), 0, 0, material.ptr));
}

static VALUE Sketchup_Materials_each(VALUE self)
{
	if (rb_block_given_p())
	{
		SUModelRef model = {DATA_PTR(self)};
		FOREACH(SUModelGetNumMaterials, SUModelGetMaterials, SUMaterialRef, model, Sketchup_Materials_Iterator, 0);
	}
	return self;
}

struct MaterialStringIterationContext
{
	VALUE key;
	SUMaterialRef material;
};

void Sketchup_Materials_Get_By_String_Iterator(SUMaterialRef material, struct MaterialStringIterationContext* material_struct)
{
	VALUE output;
	GETSTRING(SUMaterialGetName, material, output);
	if (strcmp(StringValuePtr(output), StringValuePtr(material_struct->key)) == 0)
		material_struct->material = material;
}

struct MaterialIndexIterationContext
{
	int targetIndex;
	int currentIndex;
	SUMaterialRef material;
};

void Sketchup_Materials_Get_By_Index_Iterator(SUMaterialRef material, struct MaterialIndexIterationContext* material_struct)
{
	if (material_struct->targetIndex == material_struct->currentIndex)
		material_struct->material = material;
	material_struct->currentIndex += 1;
}

static VALUE Sketchup_Materials_get(VALUE self, VALUE key)
{
	SUModelRef model = {DATA_PTR(self)};
	if (rb_type(key) == T_STRING)
	{
		struct MaterialStringIterationContext material_struct = {key, SU_INVALID};
		FOREACH(SUModelGetNumMaterials, SUModelGetMaterials, SUMaterialRef, model, Sketchup_Materials_Get_By_String_Iterator, &material_struct);
		if (SUIsInvalid(material_struct.material))
			return Qnil;
		return Data_Wrap_Struct(rb_path2class(SKETCHUP_MATERIAL), 0, 0, material_struct.material.ptr);
	}
	else
	{
		struct MaterialIndexIterationContext material_struct = {NUM2INT(key), 0, SU_INVALID};
		FOREACH(SUModelGetNumMaterials, SUModelGetMaterials, SUMaterialRef, model, Sketchup_Materials_Get_By_Index_Iterator, &material_struct);
		if (SUIsInvalid(material_struct.material))
		{
			if (material_struct.targetIndex >= material_struct.currentIndex)
				rb_raise(rb_eIndexError, "index %d out of range [0, %d)", material_struct.targetIndex, material_struct.currentIndex);
		}
		return Data_Wrap_Struct(rb_path2class(SKETCHUP_MATERIAL), 0, 0, material_struct.material.ptr);
	}
}

static VALUE Sketchup_Materials_count(VALUE self)
{
	SUModelRef model = {DATA_PTR(self)};
	size_t count = 0;
	SUModelGetNumMaterials(model, &count);
	return ULL2NUM(count);
}

static VALUE Sketchup_Materials_load(VALUE self, VALUE path)
{
	SUModelRef model = {DATA_PTR(self)};
	SUMaterialRef material = SU_INVALID;
	enum SUResult result = SUModelLoadMaterial(model, StringValuePtr(path), &material);
	if (result != SU_ERROR_NONE || SUIsInvalid(material))
		rb_raise(rb_eRuntimeError, "failed to import material to model");
	return Data_Wrap_Struct(rb_path2class(SKETCHUP_MATERIAL), 0, 0, material.ptr);
}

static VALUE Sketchup_Materials_remove(VALUE self, VALUE material)
{
	if (!rb_obj_is_kind_of(material, rb_path2class(SKETCHUP_MATERIAL)) || rb_type(material) == T_STRING)
		rb_raise(rb_eTypeError, "no implicit conversion to Color");
	SUModelRef model = {DATA_PTR(self)};
	SUMaterialRef materialRef = {DATA_PTR(Sketchup_Materials_get(self, material))};
	enum SUResult result = SUModelRemoveMaterials(model, 1, &materialRef);
	if (result != SU_ERROR_NONE)
		Qfalse;
	return Qtrue;
}

static VALUE Sketchup_Materials_unique_name(VALUE self, VALUE name)
{
	SUModelRef model = {DATA_PTR(self)};
	SUStringRef output_name = SU_INVALID;
	SUStringCreate(&output_name);
	SUModelGenerateUniqueMaterialName(model, StringValuePtr(name), &output_name);
	VALUE output;
	GETUTF8FROMSTRING(output_name, output);
	SUStringRelease(&output_name);
	return output;
}

VALUE Materials_Init(VALUE Sketchup, VALUE Sketchup_Entity)
{
	VALUE Sketchup_Materials = rb_define_class_under(Sketchup, MATERIALS, Sketchup_Entity);
	rb_undef_alloc_func(Sketchup_Materials);
	rb_include_module(Sketchup_Materials, rb_mEnumerable);
	rb_define_method(Sketchup_Materials, "each", Sketchup_Materials_each, 0);
	rb_define_method(Sketchup_Materials, "[]", Sketchup_Materials_get, 1);
	rb_define_method(Sketchup_Materials, "count", Sketchup_Materials_count, 0);
	rb_define_method(Sketchup_Materials, "length", Sketchup_Materials_count, 0);
	rb_define_method(Sketchup_Materials, "load", Sketchup_Materials_load, 1);
	rb_define_method(Sketchup_Materials, "remove", Sketchup_Materials_remove, 1);
	rb_define_method(Sketchup_Materials, "size", Sketchup_Materials_count, 0);
	rb_define_method(Sketchup_Materials, "unique_name", Sketchup_Materials_unique_name, 1);
	return Sketchup_Materials;
}