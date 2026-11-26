#include <scriptup.h>

#define AXES(self) SU_REF(SUAxesRef, self)

static VALUE axes_origin(VALUE self)
{
	struct SUPoint3D origin = {0, 0, 0};
	SUAxesGetOrigin(AXES(self), &origin);
	return su_point3d_new(origin);
}

#define DEFINE_AXIS(axis, SUFn) \
	static VALUE axes_##axis(VALUE self) \
	{ \
		struct SUVector3D v = {0, 0, 0}; \
		SUFn(AXES(self), &v); \
		return su_vector3d_new(v); \
	}
DEFINE_AXIS(xaxis, SUAxesGetXAxis)
DEFINE_AXIS(yaxis, SUAxesGetYAxis)
DEFINE_AXIS(zaxis, SUAxesGetZAxis)

static VALUE axes_axes(VALUE self)
{
	return rb_ary_new_from_args(3, axes_xaxis(self), axes_yaxis(self), axes_zaxis(self));
}

static VALUE axes_to_a(VALUE self)
{
	return rb_ary_new_from_args(4, axes_origin(self), axes_xaxis(self), axes_yaxis(self), axes_zaxis(self));
}

static VALUE axes_transformation(VALUE self)
{
	struct SUTransformation t;
	SUAxesGetTransform(AXES(self), &t);
	return su_transformation_new(t);
}

static VALUE axes_sketch_plane(VALUE self)
{
	struct SUPlane3D plane = {0, 0, 1, 0};
	SUAxesGetPlane(AXES(self), &plane);
	return rb_ary_new_from_args(4, DBL2NUM(plane.a), DBL2NUM(plane.b), DBL2NUM(plane.c), DBL2NUM(plane.d));
}

static VALUE axes_set(VALUE self, VALUE origin, VALUE xaxis, VALUE yaxis, VALUE zaxis)
{
	struct SUPoint3D o = su_to_point3d(origin);
	struct SUVector3D x = su_to_vector3d(xaxis);
	struct SUVector3D y = su_to_vector3d(yaxis);
	struct SUVector3D z = su_to_vector3d(zaxis);
	if (SUAxesSetAxesVecs(AXES(self), &x, &y, &z) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "axes must be orthogonal");
	SUAxesSetOrigin(AXES(self), &o);
	return self;
}

// Model#axes
static VALUE model_axes(VALUE model)
{
	SUAxesRef axes = SU_INVALID;
	SUModelGetAxes(SU_REF(SUModelRef, model), &axes);
	return su_wrap_or_nil(cAxes, axes.ptr);
}

void Init_axes(void)
{
	rb_define_method(cAxes, "origin", axes_origin, 0);
	rb_define_method(cAxes, "xaxis", axes_xaxis, 0);
	rb_define_method(cAxes, "yaxis", axes_yaxis, 0);
	rb_define_method(cAxes, "zaxis", axes_zaxis, 0);
	rb_define_method(cAxes, "axes", axes_axes, 0);
	rb_define_method(cAxes, "to_a", axes_to_a, 0);
	rb_define_method(cAxes, "transformation", axes_transformation, 0);
	rb_define_method(cAxes, "sketch_plane", axes_sketch_plane, 0);
	rb_define_method(cAxes, "set", axes_set, 4);
	rb_define_method(cModel, "axes", model_axes, 0);
}
