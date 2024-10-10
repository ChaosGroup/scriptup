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

VALUE Materials_Init(VALUE Sketchup, VALUE Sketchup_Entity)
{
	VALUE Sketchup_Materials = rb_define_class_under(Sketchup, MATERIALS, Sketchup_Entity);
	rb_include_module(Sketchup_Materials, rb_mEnumerable);
	rb_define_method(Sketchup_Materials, "each", Sketchup_Materials_each, 0);
	rb_define_method(Sketchup_Materials, "[]", Sketchup_Materials_get, 1);
	return Sketchup_Materials;
}