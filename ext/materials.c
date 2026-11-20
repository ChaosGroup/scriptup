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

void Init_materials(void)
{
	rb_include_module(cMaterials, rb_mEnumerable);
	rb_define_method(cMaterials, "each", materials_each, 0);
	rb_define_method(cMaterials, "[]", materials_aref, 1);
}
