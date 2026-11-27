#include <scriptup.h>

DEFINE_REF_LIST(materials_to_a, SUModelRef, SUModelGetNumMaterials, SUModelGetMaterials, SUMaterialRef, cMaterial)

static VALUE materials_each(VALUE self)
{
	return su_yield_each(self, materials_to_a(SU_REF(SUModelRef, self)));
}

static VALUE materials_aref(VALUE self, VALUE key)
{
	VALUE materials = materials_to_a(SU_REF(SUModelRef, self));
	return RB_TYPE_P(key, T_STRING) ? su_find_by(materials, "name", key) : su_fetch(materials, key);
}

static VALUE materials_length(VALUE self)
{
	size_t count = 0;
	SUModelGetNumMaterials(SU_REF(SUModelRef, self), &count);
	return SIZET2NUM(count);
}

static VALUE materials_unique_name(VALUE self, VALUE name)
{
	SUStringRef unique = SU_INVALID;
	SUStringCreate(&unique);
	SUModelGenerateUniqueMaterialName(SU_REF(SUModelRef, self), StringValueCStr(name), &unique);
	VALUE output = su_string_to_rb(unique);
	SUStringRelease(&unique);
	return output;
}

static VALUE materials_add(VALUE self, VALUE name)
{
	VALUE unique = materials_unique_name(self, name);
	SUMaterialRef material = SU_INVALID;
	SUMaterialCreate(&material);
	SUMaterialSetName(material, StringValueCStr(unique));
	SUModelAddMaterials(SU_REF(SUModelRef, self), 1, &material);
	return su_wrap(cMaterial, material.ptr);
}

static VALUE materials_load(VALUE self, VALUE path)
{
	SUMaterialRef material = SU_INVALID;
	if (SUModelLoadMaterial(SU_REF(SUModelRef, self), StringValueCStr(path), &material) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "cannot load material from %"PRIsVALUE, path);
	return su_wrap(cMaterial, material.ptr);
}

static VALUE materials_remove(VALUE self, VALUE material)
{
	SUMaterialRef ref = {su_unwrap(material, cMaterial)};
	return BOOL2RB(SUModelRemoveMaterials(SU_REF(SUModelRef, self), 1, &ref) == SU_ERROR_NONE);
}

void Init_materials(void)
{
	rb_include_module(cMaterials, rb_mEnumerable);
	rb_define_method(cMaterials, "each", materials_each, 0);
	rb_define_method(cMaterials, "[]", materials_aref, 1);
	rb_define_method(cMaterials, "at", materials_aref, 1);
	rb_define_method(cMaterials, "count", materials_length, 0);
	rb_define_method(cMaterials, "length", materials_length, 0);
	rb_define_method(cMaterials, "size", materials_length, 0);
	rb_define_method(cMaterials, "add", materials_add, 1);
	rb_define_method(cMaterials, "load", materials_load, 1);
	rb_define_method(cMaterials, "remove", materials_remove, 1);
	rb_define_method(cMaterials, "unique_name", materials_unique_name, 1);
}
