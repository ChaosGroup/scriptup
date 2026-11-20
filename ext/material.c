#include <scriptup.h>

static double alpha_from_ruby(VALUE value)
{
	return su_clamp_double(value, 0.0, 1.0);
}

static enum SUMaterialColorizeType colorize_type_from_ruby(VALUE value)
{
	int type = NUM2INT(value);
	if (type < 0)
		rb_raise(rb_eRangeError, "value cannot be negative");
	if (type > SUMaterialColorizeType_Tint)
		rb_raise(rb_eArgError, "Invalid colorization type");
	return (enum SUMaterialColorizeType)type;
}

DEFINE_STRING_GETTER(material_name, SUMaterialRef, SUMaterialGetName)
DEFINE_STRING_SETTER(material_set_name, SUMaterialRef, SUMaterialSetName)
DEFINE_STRING_GETTER(material_display_name, SUMaterialRef, SUMaterialGetNameLegacyBehavior)
DEFINE_REF_GETTER(material_texture, SUMaterialRef, SUMaterialGetTexture, SUTextureRef, cTexture)
DEFINE_ENUM_GETTER(material_materialType, SUMaterialRef, SUMaterialGetType, enum SUMaterialType)
DEFINE_DOUBLE_GETTER(material_alpha, SUMaterialRef, SUMaterialGetOpacity)
DEFINE_SETTER(material_set_alpha, SUMaterialRef, SUMaterialSetOpacity, alpha_from_ruby)
DEFINE_BOOL_GETTER(material_use_alpha_p, SUMaterialRef, SUMaterialIsDrawnTransparent)
DEFINE_ENUM_GETTER(material_colorize_type, SUMaterialRef, SUMaterialGetColorizeType, enum SUMaterialColorizeType)
DEFINE_SETTER(material_set_colorize_type, SUMaterialRef, SUMaterialSetColorizeType, colorize_type_from_ruby)

static VALUE material_set_texture(VALUE self, VALUE path)
{
	SUMaterialRef material = SU_REF(SUMaterialRef, self);
	if (NIL_P(path))
	{
		SUMaterialSetType(material, SUMaterialType_Colored);
		return Qnil;
	}
	if (!RB_TYPE_P(path, T_STRING))
		rb_raise(rb_eArgError, "Texture filename or nil required");
	SUTextureRef texture = SU_INVALID;
	SUTextureCreateFromFile(&texture, StringValueCStr(path), 1.0, 1.0);
	SUMaterialSetTexture(material, texture);
	return su_wrap(cTexture, texture.ptr);
}

static VALUE material_color(VALUE self)
{
	SUColor color = {0, 0, 0, 255};
	SUMaterialGetColor(SU_REF(SUMaterialRef, self), &color);
	return su_color_new(color);
}

static VALUE material_set_color(VALUE self, VALUE color)
{
	SUMaterialSetColor(SU_REF(SUMaterialRef, self), DATA_PTR(su_color_coerce(color)));
	return color;
}

static VALUE material_cmp(VALUE self, VALUE other)
{
	su_unwrap(other, cMaterial);
	return INT2FIX(rb_str_cmp(material_name(self), material_name(other)));
}

static VALUE material_owner_type(VALUE self)
{
	enum SUMaterialOwnerType type = SUMaterialOwnerType_DrawingElement;
	SUMaterialGetOwnerType(SU_REF(SUMaterialRef, self), &type);
	return INT2FIX(type - 1);
}

static VALUE material_colorize_deltas(VALUE self)
{
	double hue = 0.0;
	double saturation = 0.0;
	double lightness = 0.0;
	SUMaterialGetColorizeDeltas(SU_REF(SUMaterialRef, self), &hue, &saturation, &lightness);
	return rb_ary_new_from_args(3, DBL2NUM(hue), DBL2NUM(lightness), DBL2NUM(saturation));
}

static VALUE material_save_as(VALUE self, VALUE path)
{
	return BOOL2RB(SUMaterialWriteToFile(SU_REF(SUMaterialRef, self), StringValueCStr(path)) == SU_ERROR_NONE);
}

void Init_material(void)
{
	rb_define_method(cMaterial, "name", material_name, 0);
	rb_define_method(cMaterial, "name=", material_set_name, 1);
	rb_define_method(cMaterial, "display_name", material_display_name, 0);
	rb_define_method(cMaterial, "texture", material_texture, 0);
	rb_define_method(cMaterial, "texture=", material_set_texture, 1);
	rb_define_method(cMaterial, "materialType", material_materialType, 0);
	rb_define_method(cMaterial, "color", material_color, 0);
	rb_define_method(cMaterial, "color=", material_set_color, 1);
	rb_define_method(cMaterial, "<=>", material_cmp, 1);
	rb_define_method(cMaterial, "alpha", material_alpha, 0);
	rb_define_method(cMaterial, "alpha=", material_set_alpha, 1);
	rb_define_method(cMaterial, "use_alpha?", material_use_alpha_p, 0);
	rb_define_method(cMaterial, "colorize_type", material_colorize_type, 0);
	rb_define_method(cMaterial, "colorize_type=", material_set_colorize_type, 1);
	rb_define_method(cMaterial, "owner_type", material_owner_type, 0);
	rb_define_method(cMaterial, "colorize_deltas", material_colorize_deltas, 0);
	rb_define_method(cMaterial, "save_as", material_save_as, 1);
}
