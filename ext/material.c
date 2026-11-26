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

// PBR properties.
#define DEFINE_PBR_TEXTURE(name, GetFn, SetFn) \
	DEFINE_REF_GETTER(material_##name, SUMaterialRef, GetFn, SUTextureRef, cTexture) \
	static VALUE material_set_##name(VALUE self, VALUE path) \
	{ \
		SUTextureRef texture = SU_INVALID; \
		if (!NIL_P(path) && SUTextureCreateFromFile(&texture, StringValueCStr(path), 1.0, 1.0) != SU_ERROR_NONE) \
			rb_raise(rb_eArgError, "cannot load texture %"PRIsVALUE, path); \
		SetFn(SU_REF(SUMaterialRef, self), texture); \
		return path; \
	}

DEFINE_BOOL_GETTER(material_metalness_enabled_p, SUMaterialRef, SUMaterialIsMetalnessEnabled)
DEFINE_BOOL_SETTER(material_set_metalness_enabled, SUMaterialRef, SUMaterialSetMetalnessEnabled)
DEFINE_DOUBLE_GETTER(material_metallic_factor, SUMaterialRef, SUMaterialGetMetallicFactor)
DEFINE_SETTER(material_set_metallic_factor, SUMaterialRef, SUMaterialSetMetallicFactor, NUM2DBL)
DEFINE_PBR_TEXTURE(metallic_texture, SUMaterialGetMetallicTexture, SUMaterialSetMetallicTexture)
DEFINE_BOOL_GETTER(material_roughness_enabled_p, SUMaterialRef, SUMaterialIsRoughnessEnabled)
DEFINE_BOOL_SETTER(material_set_roughness_enabled, SUMaterialRef, SUMaterialSetRoughnessEnabled)
DEFINE_DOUBLE_GETTER(material_roughness_factor, SUMaterialRef, SUMaterialGetRoughnessFactor)
DEFINE_SETTER(material_set_roughness_factor, SUMaterialRef, SUMaterialSetRoughnessFactor, NUM2DBL)
DEFINE_PBR_TEXTURE(roughness_texture, SUMaterialGetRoughnessTexture, SUMaterialSetRoughnessTexture)
DEFINE_BOOL_GETTER(material_normal_enabled_p, SUMaterialRef, SUMaterialIsNormalEnabled)
DEFINE_BOOL_SETTER(material_set_normal_enabled, SUMaterialRef, SUMaterialSetNormalEnabled)
DEFINE_DOUBLE_GETTER(material_normal_scale, SUMaterialRef, SUMaterialGetNormalScale)
DEFINE_SETTER(material_set_normal_scale, SUMaterialRef, SUMaterialSetNormalScale, NUM2DBL)
DEFINE_ENUM_GETTER(material_normal_style, SUMaterialRef, SUMaterialGetNormalStyle, enum SUMaterialNormalMapStyle)
DEFINE_PBR_TEXTURE(normal_texture, SUMaterialGetNormalTexture, SUMaterialSetNormalTexture)
DEFINE_BOOL_GETTER(material_ao_enabled_p, SUMaterialRef, SUMaterialIsAOEnabled)
DEFINE_BOOL_SETTER(material_set_ao_enabled, SUMaterialRef, SUMaterialSetAOEnabled)
DEFINE_DOUBLE_GETTER(material_ao_strength, SUMaterialRef, SUMaterialGetAOStrength)
DEFINE_SETTER(material_set_ao_strength, SUMaterialRef, SUMaterialSetAOStrength, NUM2DBL)
DEFINE_PBR_TEXTURE(ao_texture, SUMaterialGetAOTexture, SUMaterialSetAOTexture)
DEFINE_ENUM_GETTER(material_workflow, SUMaterialRef, SUMaterialGetWorkflow, enum SUMaterialWorkflow)

static VALUE material_set_normal_style(VALUE self, VALUE style)
{
	if (SUMaterialSetNormalStyle(SU_REF(SUMaterialRef, self), (enum SUMaterialNormalMapStyle)NUM2INT(style)) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "invalid normal map style");
	return style;
}

void Init_material(void)
{
	static const struct { const char* name; VALUE (*fn)(ANYARGS); int argc; } pbr[] = {
		{"metalness_enabled?", RUBY_METHOD_FUNC(material_metalness_enabled_p), 0},
		{"metalness_enabled=", RUBY_METHOD_FUNC(material_set_metalness_enabled), 1},
		{"metallic_factor", RUBY_METHOD_FUNC(material_metallic_factor), 0},
		{"metallic_factor=", RUBY_METHOD_FUNC(material_set_metallic_factor), 1},
		{"metallic_texture", RUBY_METHOD_FUNC(material_metallic_texture), 0},
		{"metallic_texture=", RUBY_METHOD_FUNC(material_set_metallic_texture), 1},
		{"roughness_enabled?", RUBY_METHOD_FUNC(material_roughness_enabled_p), 0},
		{"roughness_enabled=", RUBY_METHOD_FUNC(material_set_roughness_enabled), 1},
		{"roughness_factor", RUBY_METHOD_FUNC(material_roughness_factor), 0},
		{"roughness_factor=", RUBY_METHOD_FUNC(material_set_roughness_factor), 1},
		{"roughness_texture", RUBY_METHOD_FUNC(material_roughness_texture), 0},
		{"roughness_texture=", RUBY_METHOD_FUNC(material_set_roughness_texture), 1},
		{"normal_enabled?", RUBY_METHOD_FUNC(material_normal_enabled_p), 0},
		{"normal_enabled=", RUBY_METHOD_FUNC(material_set_normal_enabled), 1},
		{"normal_scale", RUBY_METHOD_FUNC(material_normal_scale), 0},
		{"normal_scale=", RUBY_METHOD_FUNC(material_set_normal_scale), 1},
		{"normal_style", RUBY_METHOD_FUNC(material_normal_style), 0},
		{"normal_style=", RUBY_METHOD_FUNC(material_set_normal_style), 1},
		{"normal_texture", RUBY_METHOD_FUNC(material_normal_texture), 0},
		{"normal_texture=", RUBY_METHOD_FUNC(material_set_normal_texture), 1},
		{"ao_enabled?", RUBY_METHOD_FUNC(material_ao_enabled_p), 0},
		{"ao_enabled=", RUBY_METHOD_FUNC(material_set_ao_enabled), 1},
		{"ao_strength", RUBY_METHOD_FUNC(material_ao_strength), 0},
		{"ao_strength=", RUBY_METHOD_FUNC(material_set_ao_strength), 1},
		{"ao_texture", RUBY_METHOD_FUNC(material_ao_texture), 0},
		{"ao_texture=", RUBY_METHOD_FUNC(material_set_ao_texture), 1},
		{"workflow", RUBY_METHOD_FUNC(material_workflow), 0},
	};
	for (size_t i = 0; i < sizeof(pbr) / sizeof(pbr[0]); i++)
		rb_define_method(cMaterial, pbr[i].name, pbr[i].fn, pbr[i].argc);
	static const struct { const char* name; int value; } constants[] = {
		{"COLORIZE_SHIFT", SUMaterialColorizeType_Shift}, {"COLORIZE_TINT", SUMaterialColorizeType_Tint},
		{"MATERIAL_SOLID", SUMaterialType_Colored}, {"MATERIAL_TEXTURED", SUMaterialType_Textured},
		{"MATERIAL_COLORIZED_TEXTURED", SUMaterialType_ColorizedTexture},
		{"OWNER_MANAGER", 0}, {"OWNER_IMAGE", 1}, {"OWNER_LAYER", 2},
		{"NORMAL_STYLE_OPENGL", SUMaterialNormalMapStyle_OpenGL}, {"NORMAL_STYLE_DIRECTX", SUMaterialNormalMapStyle_DirectX},
		{"WORKFLOW_CLASSIC", SUMaterialWorkflow_Classic}, {"WORKFLOW_PBR_METALLIC_ROUGHNESS", SUMaterialWorkflow_PBRMetallicRoughness},
	};
	for (size_t i = 0; i < sizeof(constants) / sizeof(constants[0]); i++)
		rb_define_const(cMaterial, constants[i].name, INT2FIX(constants[i].value));
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
