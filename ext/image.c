#include <scriptup.h>

#define IMAGE(self) SU_REF(SUImageRef, self)

DEFINE_STRING_GETTER(image_path, SUImageRef, SUImageGetFileName)

static struct SUTransformation image_transform(VALUE self)
{
	struct SUTransformation t;
	SUImageGetTransform(IMAGE(self), &t);
	return t;
}

static void image_size(VALUE self, double* width, double* height)
{
	*width = 0.0;
	*height = 0.0;
	SUImageGetDimensions(IMAGE(self), width, height);
}

static VALUE image_width(VALUE self)
{
	double width, height;
	image_size(self, &width, &height);
	return su_length_new(width);
}

static VALUE image_height(VALUE self)
{
	double width, height;
	image_size(self, &width, &height);
	return su_length_new(height);
}

// Rescales the image's x and y axes to the requested world size.
static void image_resize(VALUE self, double width, double height)
{
	if (width <= 0 || height <= 0)
		rb_raise(rb_eArgError, "image size must be positive");
	double current_width, current_height;
	image_size(self, &current_width, &current_height);
	struct SUTransformation t = image_transform(self);
	double sx = width / current_width;
	double sy = height / current_height;
	for (int i = 0; i < 3; i++)
	{
		t.values[i] *= sx;
		t.values[4 + i] *= sy;
	}
	SUImageSetTransform(IMAGE(self), &t);
}

static VALUE image_set_width(VALUE self, VALUE width)
{
	double w, h;
	image_size(self, &w, &h);
	double new_width = NUM2DBL(width);
	image_resize(self, new_width, h * new_width / w);
	return width;
}

static VALUE image_set_height(VALUE self, VALUE height)
{
	double w, h;
	image_size(self, &w, &h);
	double new_height = NUM2DBL(height);
	image_resize(self, w * new_height / h, new_height);
	return height;
}

// image.size = width, height arrives as one array.
static VALUE image_set_size(int argc, VALUE* argv, VALUE self)
{
	VALUE size = argc == 1 ? argv[0] : rb_ary_new_from_values(argc, argv);
	double wh[2];
	su_array_to_xy(size, wh);
	image_resize(self, wh[0], wh[1]);
	return size;
}

static VALUE image_image_rep(VALUE self)
{
	SUImageRepRef rep = SU_INVALID;
	SUImageRepCreate(&rep);
	if (SUImageGetImageRep(IMAGE(self), &rep) != SU_ERROR_NONE)
	{
		SUImageRepRelease(&rep);
		return Qnil;
	}
	return su_image_rep_new(rep);
}

// Pixel sizes come from the image rep (SUImageGetPixelDimensions is deprecated).
static VALUE image_pixelwidth(VALUE self)
{
	VALUE rep = image_image_rep(self);
	return NIL_P(rep) ? INT2FIX(0) : rb_funcall(rep, (rb_intern)("width"), 0);
}

static VALUE image_pixelheight(VALUE self)
{
	VALUE rep = image_image_rep(self);
	return NIL_P(rep) ? INT2FIX(0) : rb_funcall(rep, (rb_intern)("height"), 0);
}

static VALUE image_transformation(VALUE self)
{
	return su_transformation_new(image_transform(self));
}

static VALUE image_set_transformation(VALUE self, VALUE transformation)
{
	struct SUTransformation t = su_to_transformation(transformation);
	SUImageSetTransform(IMAGE(self), &t);
	return transformation;
}

static VALUE image_transform_bang(VALUE self, VALUE transformation)
{
	struct SUTransformation by = su_to_transformation(transformation);
	struct SUTransformation current = image_transform(self);
	struct SUTransformation result;
	SUTransformationMultiply(&by, &current, &result);
	SUImageSetTransform(IMAGE(self), &result);
	return self;
}

static VALUE image_origin(VALUE self)
{
	struct SUTransformation t = image_transform(self);
	struct SUPoint3D origin;
	SUTransformationGetOrigin(&t, &origin);
	return su_point3d_new(origin);
}

static VALUE image_set_origin(VALUE self, VALUE point)
{
	struct SUPoint3D p = su_to_point3d(point);
	struct SUTransformation t = image_transform(self);
	t.values[12] = p.x;
	t.values[13] = p.y;
	t.values[14] = p.z;
	SUImageSetTransform(IMAGE(self), &t);
	return point;
}

static VALUE image_normal(VALUE self)
{
	struct SUTransformation t = image_transform(self);
	struct SUVector3D z;
	SUTransformationGetZAxis(&t, &z);
	SUVector3DNormalize(&z);
	return su_vector3d_new(z);
}

static VALUE image_zrotation(VALUE self)
{
	struct SUTransformation t = image_transform(self);
	double angle = 0.0;
	SUTransformationGetZRotation(&t, &angle);
	return DBL2NUM(angle);
}

void Init_image(void)
{
	rb_define_method(cImage, "path", image_path, 0);
	rb_define_method(cImage, "width", image_width, 0);
	rb_define_method(cImage, "height", image_height, 0);
	rb_define_method(cImage, "width=", image_set_width, 1);
	rb_define_method(cImage, "height=", image_set_height, 1);
	rb_define_method(cImage, "size=", image_set_size, -1);
	rb_define_method(cImage, "pixelwidth", image_pixelwidth, 0);
	rb_define_method(cImage, "pixelheight", image_pixelheight, 0);
	rb_define_method(cImage, "image_rep", image_image_rep, 0);
	rb_define_method(cImage, "transformation", image_transformation, 0);
	rb_define_method(cImage, "transformation=", image_set_transformation, 1);
	rb_define_method(cImage, "transform!", image_transform_bang, 1);
	rb_define_method(cImage, "origin", image_origin, 0);
	rb_define_method(cImage, "origin=", image_set_origin, 1);
	rb_define_method(cImage, "normal", image_normal, 0);
	rb_define_method(cImage, "zrotation", image_zrotation, 0);
}
