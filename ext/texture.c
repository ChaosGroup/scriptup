#include <scriptup.h>

struct texture_dimensions
{
	size_t width;
	size_t height;
	double s_scale;
	double t_scale;
};

static struct texture_dimensions texture_dimensions(VALUE self)
{
	struct texture_dimensions d = {0, 0, 0.0, 0.0};
	SUTextureGetDimensions(SU_REF(SUTextureRef, self), &d.width, &d.height, &d.s_scale, &d.t_scale);
	return d;
}

DEFINE_STRING_GETTER(texture_filename, SUTextureRef, SUTextureGetFileName)

static void image_rep_free(void* ptr)
{
	SUImageRepRef image = {ptr};
	SUImageRepRelease(&image);
}

// The caller owns the image rep, so it is released when the Ruby object is collected.
static VALUE texture_image_rep(int argc, VALUE* argv, VALUE self)
{
	VALUE colorized;
	rb_scan_args(argc, argv, "01", &colorized);
	SUImageRepRef image = SU_INVALID;
	SUImageRepCreate(&image);
	SUTextureRef texture = SU_REF(SUTextureRef, self);
	enum SUResult result = RTEST(colorized) ? SUTextureGetColorizedImageRep(texture, &image) : SUTextureGetImageRep(texture, &image);
	if (result != SU_ERROR_NONE)
	{
		SUImageRepRelease(&image);
		return Qnil;
	}
	return Data_Wrap_Struct(cImageRep, 0, image_rep_free, image.ptr);
}

static VALUE texture_average_color(VALUE self)
{
	SUColor color = {0, 0, 0, 255};
	if (SUTextureGetAverageColor(SU_REF(SUTextureRef, self), &color) != SU_ERROR_NONE)
		return Qnil;
	return su_color_new(color);
}

static VALUE texture_image_width(VALUE self) { return SIZET2NUM(texture_dimensions(self).width); }
static VALUE texture_image_height(VALUE self) { return SIZET2NUM(texture_dimensions(self).height); }
static VALUE texture_width(VALUE self) { return DBL2NUM(1.0 / texture_dimensions(self).s_scale); }
static VALUE texture_height(VALUE self) { return DBL2NUM(1.0 / texture_dimensions(self).t_scale); }

static VALUE texture_valid_p(VALUE self)
{
	return BOOL2RB(DATA_PTR(self) != NULL);
}

static VALUE texture_write(VALUE self, VALUE path, VALUE colorize)
{
	SUTextureRef texture = SU_REF(SUTextureRef, self);
	const char* file = StringValueCStr(path);
	enum SUResult result = RTEST(colorize) ? SUTextureWriteToFile(texture, file) : SUTextureWriteOriginalToFile(texture, file);
	return BOOL2RB(result == SU_ERROR_NONE);
}

void Init_texture(void)
{
	rb_define_method(cTexture, "average_color", texture_average_color, 0);
	rb_define_method(cTexture, "filename", texture_filename, 0);
	rb_define_method(cTexture, "image_height", texture_image_height, 0);
	rb_define_method(cTexture, "height", texture_height, 0);
	rb_define_method(cTexture, "image_width", texture_image_width, 0);
	rb_define_method(cTexture, "width", texture_width, 0);
	rb_define_method(cTexture, "image_rep", texture_image_rep, -1);
	rb_define_method(cTexture, "valid?", texture_valid_p, 0);
	rb_define_method(cTexture, "write", texture_write, 2);
}
