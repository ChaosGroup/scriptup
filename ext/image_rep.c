#include <scriptup.h>

#define IMAGE(self) SU_REF(SUImageRepRef, self)

static void imagerep_free(void* ptr)
{
	SUImageRepRef image = {ptr};
	if (ptr)
		SUImageRepRelease(&image);
}

static VALUE imagerep_alloc(VALUE klass)
{
	SUImageRepRef image = SU_INVALID;
	SUImageRepCreate(&image);
	return Data_Wrap_Struct(klass, 0, imagerep_free, image.ptr);
}

// Takes ownership of a caller-owned image rep.
VALUE su_image_rep_new(SUImageRepRef image)
{
	return Data_Wrap_Struct(cImageRep, 0, imagerep_free, image.ptr);
}

static VALUE imagerep_load_file(VALUE self, VALUE path)
{
	if (SUImageRepLoadFile(IMAGE(self), StringValueCStr(path)) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "cannot load image %"PRIsVALUE, path);
	return self;
}

static VALUE imagerep_initialize(int argc, VALUE* argv, VALUE self)
{
	VALUE path;
	rb_scan_args(argc, argv, "01", &path);
	if (!NIL_P(path))
		imagerep_load_file(self, path);
	return self;
}

static VALUE imagerep_save_file(VALUE self, VALUE path)
{
	if (SUImageRepSaveToFile(IMAGE(self), StringValueCStr(path)) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "cannot save image to %"PRIsVALUE, path);
	return self;
}

static void dimensions(VALUE self, size_t* width, size_t* height)
{
	*width = 0;
	*height = 0;
	SUImageRepGetPixelDimensions(IMAGE(self), width, height);
}

static VALUE imagerep_width(VALUE self)
{
	size_t width, height;
	dimensions(self, &width, &height);
	return SIZET2NUM(width);
}

static VALUE imagerep_height(VALUE self)
{
	size_t width, height;
	dimensions(self, &width, &height);
	return SIZET2NUM(height);
}

DEFINE_SIZE_GETTER(imagerep_row_padding, SUImageRepRef, SUImageRepGetRowPadding)

static void data_size(VALUE self, size_t* size, size_t* bits)
{
	*size = 0;
	*bits = 0;
	SUImageRepGetDataSize(IMAGE(self), size, bits);
}

static VALUE imagerep_size(VALUE self)
{
	size_t size, bits;
	data_size(self, &size, &bits);
	return SIZET2NUM(size);
}

static VALUE imagerep_bits_per_pixel(VALUE self)
{
	size_t size, bits;
	data_size(self, &size, &bits);
	return SIZET2NUM(bits);
}

static VALUE imagerep_data(VALUE self)
{
	size_t size, bits;
	data_size(self, &size, &bits);
	if (size == 0)
		return Qnil;
	VALUE data = rb_str_new(NULL, (long)size);
	SUImageRepGetData(IMAGE(self), size, (SUByte*)RSTRING_PTR(data));
	return data;
}

static VALUE imagerep_colors(VALUE self)
{
	size_t width, height;
	dimensions(self, &width, &height);
	if (width == 0 || height == 0)
		return Qnil;
	SUColor* colors = ZALLOC_N(SUColor, width * height);
	SUImageRepGetDataAsColors(IMAGE(self), colors);
	VALUE ary = rb_ary_new_capa((long)(width * height));
	for (size_t i = 0; i < width * height; i++)
		rb_ary_push(ary, su_color_new(colors[i]));
	xfree(colors);
	return ary;
}

static VALUE imagerep_color_at_uv(int argc, VALUE* argv, VALUE self)
{
	VALUE u, v, bilinear;
	rb_scan_args(argc, argv, "21", &u, &v, &bilinear);
	SUColor color = {0, 0, 0, 255};
	if (SUImageRepGetColorAtUV(IMAGE(self), NUM2DBL(u), NUM2DBL(v), RTEST(bilinear), &color) != SU_ERROR_NONE)
		return Qnil;
	return su_color_new(color);
}

static VALUE imagerep_set_data(VALUE self, VALUE width, VALUE height, VALUE bits, VALUE padding, VALUE pixels)
{
	size_t w = NUM2SIZET(width);
	size_t h = NUM2SIZET(height);
	size_t bpp = NUM2SIZET(bits);
	size_t pad = NUM2SIZET(padding);
	StringValue(pixels);
	size_t expected = (w * bpp / 8 + pad) * h;
	if ((size_t)RSTRING_LEN(pixels) < expected)
		rb_raise(rb_eArgError, "pixel data is too short: expected %zu bytes", expected);
	if (SUImageRepSetData(IMAGE(self), w, h, bpp, pad, (const SUByte*)RSTRING_PTR(pixels)) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "invalid image data");
	return self;
}

void Init_image_rep(void)
{
	rb_define_alloc_func(cImageRep, imagerep_alloc);
	rb_define_method(cImageRep, "initialize", imagerep_initialize, -1);
	rb_define_method(cImageRep, "load_file", imagerep_load_file, 1);
	rb_define_method(cImageRep, "save_file", imagerep_save_file, 1);
	rb_define_method(cImageRep, "width", imagerep_width, 0);
	rb_define_method(cImageRep, "height", imagerep_height, 0);
	rb_define_method(cImageRep, "row_padding", imagerep_row_padding, 0);
	rb_define_method(cImageRep, "size", imagerep_size, 0);
	rb_define_method(cImageRep, "bits_per_pixel", imagerep_bits_per_pixel, 0);
	rb_define_method(cImageRep, "data", imagerep_data, 0);
	rb_define_method(cImageRep, "colors", imagerep_colors, 0);
	rb_define_method(cImageRep, "color_at_uv", imagerep_color_at_uv, -1);
	rb_define_method(cImageRep, "set_data", imagerep_set_data, 5);
}
