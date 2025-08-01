#include <stdbool.h>
#include <texture.h>
#include <SketchUpAPI/sketchup.h>
#include <not_implemented.h>
#include <utils.h>

static VALUE Sketchup_Texture_average_color(VALUE self)
{
	SUTextureRef texture = {DATA_PTR(self)};
	VALUE c = rb_obj_alloc(rb_path2class(SKETCHUP_COLOR));
	SUColor* color = DATA_PTR(c);
	enum SUResult result = SUTextureGetAverageColor(texture, color);
	if (result != SU_ERROR_NONE)
		return Qnil;
	return c;
}

static VALUE Sketchup_Texture_filename(VALUE self)
{
	SUTextureRef texture = {DATA_PTR(self)};
	VALUE output;
	GETSTRING(SUTextureGetFileName, texture, output);
	return output;
}

static VALUE Sketchup_Texture_image_height(VALUE self)
{
	SUTextureRef texture = {DATA_PTR(self)};
	size_t width = 0;
	size_t height = 0;
	double s_scale = 0.0;
	double t_scale = 0.0;
	SUTextureGetDimensions(texture, &width, &height, &s_scale, &t_scale);
	return ULL2NUM(height);
}

static VALUE Sketchup_Texture_height(VALUE self)
{
	SUTextureRef texture = {DATA_PTR(self)};
	size_t width = 0;
	size_t height = 0;
	double s_scale = 0.0;
	double t_scale = 0.0;
	SUTextureGetDimensions(texture, &width, &height, &s_scale, &t_scale);
	return DBL2NUM(1.0 / t_scale);
}

static VALUE Sketchup_Texture_width(VALUE self)
{
	SUTextureRef texture = {DATA_PTR(self)};
	size_t width = 0;
	size_t height = 0;
	double s_scale = 0.0;
	double t_scale = 0.0;
	SUTextureGetDimensions(texture, &width, &height, &s_scale, &t_scale);
	return DBL2NUM(1.0 / s_scale);
}

static VALUE Sketchup_Texture_image_width(VALUE self)
{
	SUTextureRef texture = {DATA_PTR(self)};
	size_t width = 0;
	size_t height = 0;
	double s_scale = 0.0;
	double t_scale = 0.0;
	SUTextureGetDimensions(texture, &width, &height, &s_scale, &t_scale);
	return ULL2NUM(width);
}

static VALUE Sketchup_Texture_image_rep(VALUE self)
{
	SUTextureRef texture = {DATA_PTR(self)};
	SUImageRepRef image = SU_INVALID;
	SUTextureGetImageRep(texture, &image);
	return Data_Wrap_Struct(rb_path2class(SKETCHUP_IMAGEREP), 0, 0, image.ptr);
}

static VALUE Sketchup_Texture_Get_valid(VALUE self)
{
	SUTextureRef texture = {DATA_PTR(self)};
	return SUIsValid(texture) ? Qtrue : Qfalse;
}

static VALUE Sketchup_Texture_write(VALUE self, VALUE path, VALUE colorize)
{
	SUTextureRef texture = {DATA_PTR(self)};
	const char* path_ptr = StringValuePtr(path);
	enum SUResult result = SU_ERROR_UNSUPPORTED;
	if (RTEST(colorize))
		result = SUTextureWriteToFile(texture, path_ptr);
	else
		result = SUTextureWriteOriginalToFile(texture, path_ptr);
	return result != SU_ERROR_NONE ? Qfalse : Qtrue;
}

void Texture_Init(VALUE Sketchup, VALUE Sketchup_Entity)
{
	VALUE Sketchup_Texture = rb_define_class_under(Sketchup, TEXTURE, Sketchup_Entity);
	rb_undef_alloc_func(Sketchup_Texture);
	rb_define_method(Sketchup_Texture, "average_color", Sketchup_Texture_average_color, 0);
	rb_define_method(Sketchup_Texture, "filename", Sketchup_Texture_filename, 0);
	rb_define_method(Sketchup_Texture, "image_height", Sketchup_Texture_image_height, 0);
	rb_define_method(Sketchup_Texture, "height", Sketchup_Texture_height, 0);
	rb_define_method(Sketchup_Texture, "image_width", Sketchup_Texture_image_width, 0);
	rb_define_method(Sketchup_Texture, "width", Sketchup_Texture_width, 0);
	rb_define_method(Sketchup_Texture, "image_rep", Sketchup_Texture_image_rep, 0);
	rb_define_method(Sketchup_Texture, "valid?", Sketchup_Texture_Get_valid, 0);
	rb_define_method(Sketchup_Texture, "write", Sketchup_Texture_write, 2);
}
