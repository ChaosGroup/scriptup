#include <stdbool.h>
#include <material.h>
#include <SketchUpAPI/sketchup.h>
#include <utils.h>

static int GETCOLOR(VALUE obj)
{
	int color = NUM2INT(obj);
	if (color > 255)
		return 255;
	else if (color < 0)
		return 0;
	else
		return color;
}

static float GETALPHA(VALUE obj)
{
	float alpha = NUM2DBL(obj);
	if (alpha > 1.0)
		return 1.0;
	else if (alpha < 0.0)
		return 0.0;
	else
		return alpha;
}

static void dealloc(SUColor* ptr)
{
	free(ptr);
}

static VALUE VALUE2COLOR(VALUE obj)
{
	if (rb_obj_is_kind_of(obj, rb_path2class(SKETCHUP_COLOR)))
		return obj;
	else if (rb_type(obj) == T_ARRAY)
	{
		if (RARRAY_LEN(obj) >= 3)
		{
			VALUE* start_arr = RARRAY_PTR(obj);
			VALUE c = rb_obj_alloc(rb_path2class(SKETCHUP_COLOR));
			SUColor* color = DATA_PTR(c);
			color->red = GETCOLOR(start_arr[0]);
			color->green = GETCOLOR(start_arr[1]);
			color->blue = GETCOLOR(start_arr[2]);
			color->alpha = 255;
			return c;
		}
		else
			rb_raise(rb_eArgError, "At least 3 values required");
	}
	else
		rb_raise(rb_eTypeError, "no implicit conversion to Color");
}

static VALUE VALUE2COLORIZATION(VALUE obj)
{
	int colorization = NUM2INT(obj);
	if (colorization < 0)
		rb_raise(rb_eRangeError, "value cannot be negative");
	else if (colorization > 2)
		rb_raise(rb_eArgError, "Invalid colorization type");
	else
		return colorization;
}

static VALUE Sketchup_Material_Get_name(VALUE self)
{
	SUMaterialRef material = {DATA_PTR(self)};
	VALUE output;
	GETSTRING(SUMaterialGetName, material, output);
	return output;
}

static VALUE Sketchup_Material_Set_name(VALUE self, VALUE name)
{
	SUMaterialRef material = {DATA_PTR(self)};
	SUMaterialSetName(material, StringValuePtr(name));
	return name;
}

static VALUE Sketchup_Material_Get_texture(VALUE self)
{
	SUMaterialRef material = {DATA_PTR(self)};
	SUTextureRef texture = SU_INVALID;
	SUMaterialGetTexture(material, &texture);
	if (SUIsValid(texture))
		return Data_Wrap_Struct(rb_path2class(SKETCHUP_TEXTURE), 0, 0, texture.ptr);
	return Qnil;
}

static VALUE Sketchup_Material_Set_texture(VALUE self, VALUE texturePath)
{
	SUMaterialRef material = {DATA_PTR(self)};
	if (rb_type(texturePath) == T_NIL)
	{
		SUMaterialSetType(material, 0);
		return Qnil;
	}
	else if (rb_type(texturePath) == T_STRING)
	{
		SUTextureRef suTexture = SU_INVALID;
		SUTextureCreateFromFile(&suTexture, StringValuePtr(texturePath), 1.0, 1.0);
		SUMaterialSetTexture(material, suTexture);
		return Data_Wrap_Struct(rb_path2class(SKETCHUP_TEXTURE), 0, 0, suTexture.ptr);
	}
	else
	{
		rb_raise(rb_eArgError, "Texture filename or nil required");
	}
}

static VALUE Sketchup_Material_materialType(VALUE self)
{
	SUMaterialRef material = {DATA_PTR(self)};
	enum SUMaterialType suMaterialType = SUMaterialType_Colored;
	SUMaterialGetType(material, &suMaterialType);
	return INT2FIX(suMaterialType);
}

static VALUE Sketchup_Material_Get_color(VALUE self)
{
	SUMaterialRef material = {DATA_PTR(self)};
	VALUE color = rb_obj_alloc(rb_path2class(SKETCHUP_COLOR));
	SUMaterialGetColor(material, DATA_PTR(color));
	return color;
}

static VALUE Sketchup_Material_Set_color(VALUE self, VALUE color)
{
	SUMaterialRef material = {DATA_PTR(self)};
	VALUE valueColor = VALUE2COLOR(color);
	return color;
}

static VALUE Sketchup_Material_Name_equ(VALUE self, VALUE other)
{
	if (!rb_obj_is_kind_of(other, rb_path2class(SKETCHUP_MATERIAL)))
		rb_raise(rb_eStandardError, "Wrong type of object given");
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcompound-token-split-by-macro"
	return rb_funcall(Sketchup_Material_Get_name(self), rb_intern("<=>"), 1, Sketchup_Material_Get_name(other));
#pragma clang diagnostic pop
}

static VALUE Sketchup_Material_Get_alpha(VALUE self)
{
	SUMaterialRef material = {DATA_PTR(self)};
	double alpha = 0.0;
	SUMaterialGetOpacity(material, &alpha);
	return DBL2NUM(alpha);
}

static VALUE Sketchup_Material_Set_alpha(VALUE self, VALUE alpha)
{
	SUMaterialRef material = {DATA_PTR(self)};
	SUMaterialSetOpacity(material, GETALPHA(alpha));
	return alpha;
}

static VALUE Sketchup_Material_Use_alpha(VALUE self)
{
	SUMaterialRef material = {DATA_PTR(self)};
	bool transparency = false;
	SUMaterialIsDrawnTransparent(material, &transparency);
	return transparency ? Qtrue : Qfalse;
}

static VALUE Sketchup_Material_Get_colorize_type(VALUE self)
{
	SUMaterialRef material = {DATA_PTR(self)};
	enum SUMaterialColorizeType type = SU_INVALID;
	SUMaterialGetColorizeType(material, &type);
	return INT2FIX(type);
}

static VALUE Sketchup_Material_Set_colorize_type(VALUE self, VALUE type)
{
	SUMaterialRef material = {DATA_PTR(self)};
	SUMaterialSetColorizeType(material, VALUE2COLORIZATION(type));
	return type;
}

static VALUE Sketchup_Material_owner_type(VALUE self)
{
	SUMaterialRef material = {DATA_PTR(self)};
	enum SUMaterialOwnerType type = SUMaterialOwnerType_DrawingElement;
	SUMaterialGetOwnerType(material, &type);
	return INT2FIX(type - 1);
}

static VALUE Sketchup_Material_colorize_deltas(VALUE self)
{
	SUMaterialRef material = {DATA_PTR(self)};
	double hue = 0.0;
	double saturation = 0.0;
	double lightness = 0.0;
	SUMaterialGetColorizeDeltas(material, &hue, &saturation, &lightness);
	VALUE ary = rb_ary_new2(3);
	rb_ary_push(ary, DBL2NUM(hue));
	rb_ary_push(ary, DBL2NUM(lightness));
	rb_ary_push(ary, DBL2NUM(saturation));
	return ary;
}

static VALUE Sketchup_Material_save_as(VALUE self, VALUE path)
{
	SUMaterialRef material = {DATA_PTR(self)};
	SUMaterialWriteToFile(material, StringValuePtr(path));
	return Qtrue;
}

static VALUE Sketchup_Material_display_name(VALUE self)
{
	SUMaterialRef material = {DATA_PTR(self)};
	VALUE output;
	GETSTRING(SUMaterialGetNameLegacyBehavior, material, output);
	return output;
}

void Material_Init(VALUE Sketchup, VALUE Sketchup_Entity)
{
	VALUE Sketchup_Material = rb_define_class_under(Sketchup, MATERIAL, Sketchup_Entity);
	rb_undef_alloc_func(Sketchup_Material);
	rb_define_method(Sketchup_Material, "name", Sketchup_Material_Get_name, 0);
	rb_define_method(Sketchup_Material, "name=", Sketchup_Material_Set_name, 1);
	rb_define_method(Sketchup_Material, "texture", Sketchup_Material_Get_texture, 0);
	rb_define_method(Sketchup_Material, "texture=", Sketchup_Material_Set_texture, 1);
	rb_define_method(Sketchup_Material, "materialType", Sketchup_Material_materialType, 0);
	rb_define_method(Sketchup_Material, "color", Sketchup_Material_Get_color, 0);
	rb_define_method(Sketchup_Material, "color=", Sketchup_Material_Set_color, 1);
	rb_define_method(Sketchup_Material, "<=>", Sketchup_Material_Name_equ, 1);
	rb_define_method(Sketchup_Material, "alpha", Sketchup_Material_Get_alpha, 0);
	rb_define_method(Sketchup_Material, "alpha=", Sketchup_Material_Set_alpha, 1);
	rb_define_method(Sketchup_Material, "use_alpha?", Sketchup_Material_Use_alpha, 0);
	rb_define_method(Sketchup_Material, "colorize_type", Sketchup_Material_Get_colorize_type, 0);
	rb_define_method(Sketchup_Material, "colorize_type=", Sketchup_Material_Set_colorize_type, 1);
	rb_define_method(Sketchup_Material, "owner_type", Sketchup_Material_owner_type, 0);
	rb_define_method(Sketchup_Material, "colorize_deltas", Sketchup_Material_colorize_deltas, 0);
	rb_define_method(Sketchup_Material, "save_as", Sketchup_Material_save_as, 1);
	rb_define_method(Sketchup_Material, "display_name", Sketchup_Material_display_name, 0);
}
