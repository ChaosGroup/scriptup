#include <scriptup.h>

#define TR(obj) ((struct SUTransformation*)DATA_PTR(obj))

static const struct SUTransformation identity = {{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}};

static VALUE transformation_alloc(VALUE klass)
{
	struct SUTransformation* t;
	VALUE obj = Data_Make_Struct(klass, struct SUTransformation, 0, RUBY_DEFAULT_FREE, t);
	*t = identity;
	return obj;
}

VALUE su_transformation_new(struct SUTransformation value)
{
	VALUE obj = transformation_alloc(cTransformation);
	*TR(obj) = value;
	return obj;
}

struct SUTransformation su_to_transformation(VALUE obj)
{
	return *(struct SUTransformation*)su_unwrap(obj, cTransformation);
}

static struct SUTransformation axes_transformation(VALUE origin, VALUE xaxis, VALUE yaxis, VALUE zaxis)
{
	struct SUPoint3D o = su_to_point3d(origin);
	struct SUVector3D x = su_to_vector3d(xaxis);
	struct SUVector3D y = su_to_vector3d(yaxis);
	struct SUVector3D z;
	if (NIL_P(zaxis))
		SUVector3DCross(&x, &y, &z);
	else
		z = su_to_vector3d(zaxis);
	if (SUVector3DNormalize(&x) != SU_ERROR_NONE || SUVector3DNormalize(&y) != SU_ERROR_NONE ||
		SUVector3DNormalize(&z) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "invalid axes");
	struct SUTransformation t = {{x.x, x.y, x.z, 0, y.x, y.y, y.z, 0, z.x, z.y, z.z, 0, o.x, o.y, o.z, 1}};
	return t;
}

// Applies t about point: translate(point) * t * translate(-point).
static struct SUTransformation about_point_fn(struct SUTransformation t, struct SUPoint3D point)
{
	struct SUTransformation to = identity;
	struct SUTransformation back = identity;
	struct SUVector3D v = {point.x, point.y, point.z};
	SUTransformationTranslation(&to, &v);
	SUVector3DReverse(&v);
	SUTransformationTranslation(&back, &v);
	struct SUTransformation tmp;
	struct SUTransformation result;
	SUTransformationMultiply(&t, &back, &tmp);
	SUTransformationMultiply(&to, &tmp, &result);
	return result;
}

static struct SUTransformation rotation(VALUE point, VALUE axis, VALUE angle)
{
	struct SUPoint3D origin = {0, 0, 0};
	struct SUVector3D v = su_to_vector3d(axis);
	bool valid = false;
	SUVector3DIsValid(&v, &valid);
	if (!valid)
		rb_raise(rb_eArgError, "invalid rotation axis");
	struct SUTransformation t;
	SUTransformationRotation(&t, &origin, &v, NUM2DBL(angle));
	return about_point_fn(t, su_to_point3d(point));
}

// Single argument forms shared by new and set!.
static struct SUTransformation transformation_from(VALUE arg)
{
	struct SUTransformation t = identity;
	if (rb_obj_is_kind_of(arg, cTransformation))
		return *TR(arg);
	if (rb_obj_is_kind_of(arg, cPoint3d))
	{
		struct SUPoint3D p = su_to_point3d(arg);
		struct SUVector3D v = {p.x, p.y, p.z};
		SUTransformationTranslation(&t, &v);
	}
	else if (rb_obj_is_kind_of(arg, cVector3d))
	{
		struct SUVector3D v = su_to_vector3d(arg);
		SUTransformationTranslation(&t, &v);
	}
	else if (RB_TYPE_P(arg, T_ARRAY) && RARRAY_LEN(arg) == 16)
	{
		for (long i = 0; i < 16; i++)
			t.values[i] = NUM2DBL(RARRAY_AREF(arg, i));
	}
	else if (RB_TYPE_P(arg, T_ARRAY))
	{
		double xyz[3];
		su_array_to_xyz(arg, xyz);
		struct SUVector3D v = {xyz[0], xyz[1], xyz[2]};
		SUTransformationTranslation(&t, &v);
	}
	else if (rb_obj_is_kind_of(arg, rb_cNumeric))
		SUTransformationScale(&t, NUM2DBL(arg));
	else
		rb_raise(rb_eTypeError, "cannot convert %"PRIsVALUE" to a transformation", rb_obj_class(arg));
	return t;
}

static VALUE transformation_initialize(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 0, 4);
	struct SUTransformation* t = TR(self);
	switch (argc)
	{
		case 1:
			*t = transformation_from(argv[0]);
			break;
		case 2:
		{
			struct SUVector3D normal = su_to_vector3d(argv[1]);
			struct SUVector3D x;
			struct SUVector3D y;
			SUVector3DGetArbitraryAxes(&normal, &x, &y);
			*t = axes_transformation(argv[0], su_vector3d_new(x), su_vector3d_new(y), argv[1]);
			break;
		}
		case 3:
			if (rb_obj_is_kind_of(argv[2], rb_cNumeric))
				*t = rotation(argv[0], argv[1], argv[2]);
			else
				*t = axes_transformation(argv[0], argv[1], argv[2], Qnil);
			break;
		case 4:
			*t = axes_transformation(argv[3], argv[0], argv[1], argv[2]);
			break;
	}
	return self;
}

static VALUE transformation_initialize_copy(VALUE self, VALUE other)
{
	*TR(self) = *TR(other);
	return self;
}

static VALUE transformation_set_bang(VALUE self, VALUE arg)
{
	*TR(self) = transformation_from(arg);
	return self;
}

static VALUE transformation_s_axes(int argc, VALUE* argv, VALUE klass)
{
	rb_check_arity(argc, 3, 4);
	return su_transformation_new(axes_transformation(argv[0], argv[1], argv[2], argc == 4 ? argv[3] : Qnil));
}

static VALUE transformation_s_interpolate(VALUE klass, VALUE t1, VALUE t2, VALUE weight)
{
	struct SUTransformation a = su_to_transformation(t1);
	struct SUTransformation b = su_to_transformation(t2);
	struct SUTransformation t;
	SUTransformationInterpolate(&t, &a, &b, NUM2DBL(weight));
	return su_transformation_new(t);
}

static VALUE transformation_s_rotation(VALUE klass, VALUE point, VALUE axis, VALUE angle)
{
	return su_transformation_new(rotation(point, axis, angle));
}

static VALUE transformation_s_scaling(int argc, VALUE* argv, VALUE klass)
{
	rb_check_arity(argc, 1, 4);
	struct SUTransformation t = identity;
	bool about_point = rb_obj_is_kind_of(argv[0], cPoint3d) || RB_TYPE_P(argv[0], T_ARRAY);
	struct SUPoint3D point = about_point ? su_to_point3d(argv[0]) : (struct SUPoint3D){0, 0, 0};
	int scales = about_point ? argc - 1 : argc;
	const VALUE* s = about_point ? argv + 1 : argv;
	if (scales == 1)
		SUTransformationScale(&t, NUM2DBL(s[0]));
	else if (scales == 3)
		SUTransformationNonUniformScale(&t, NUM2DBL(s[0]), NUM2DBL(s[1]), NUM2DBL(s[2]));
	else
		rb_raise(rb_eArgError, "expected 1 or 3 scale values");
	return su_transformation_new(about_point ? about_point_fn(t, point) : t);
}

static VALUE transformation_s_translation(VALUE klass, VALUE vector)
{
	struct SUTransformation t = identity;
	struct SUVector3D v;
	if (rb_obj_is_kind_of(vector, cPoint3d))
	{
		struct SUPoint3D p = su_to_point3d(vector);
		v = (struct SUVector3D){p.x, p.y, p.z};
	}
	else
		v = su_to_vector3d(vector);
	SUTransformationTranslation(&t, &v);
	return su_transformation_new(t);
}

static VALUE plane_to_a(struct SUPlane3D plane)
{
	return rb_ary_new_from_args(4, DBL2NUM(plane.a), DBL2NUM(plane.b), DBL2NUM(plane.c), DBL2NUM(plane.d));
}

static VALUE transformation_mul(VALUE self, VALUE other)
{
	struct SUTransformation* t = TR(self);
	if (rb_obj_is_kind_of(other, cTransformation))
	{
		struct SUTransformation result;
		SUTransformationMultiply(t, TR(other), &result);
		return su_transformation_new(result);
	}
	if (rb_obj_is_kind_of(other, cPoint3d))
	{
		struct SUPoint3D p = su_to_point3d(other);
		SUPoint3DTransform(t, &p);
		return su_point3d_new(p);
	}
	if (rb_obj_is_kind_of(other, cVector3d))
	{
		struct SUVector3D v = su_to_vector3d(other);
		SUVector3DTransform(t, &v);
		return su_vector3d_new(v);
	}
	if (RB_TYPE_P(other, T_ARRAY) && (RARRAY_LEN(other) == 2 || RARRAY_LEN(other) == 4) &&
		!(RARRAY_LEN(other) == 2 && rb_obj_is_kind_of(RARRAY_AREF(other, 0), rb_cNumeric)))
	{
		struct SUPlane3D plane = su_to_plane3d(other);
		SUPlane3DTransform(t, &plane);
		return plane_to_a(plane);
	}
	struct SUPoint3D p = su_to_point3d(other);
	SUPoint3DTransform(t, &p);
	return rb_ary_new_from_args(3, DBL2NUM(p.x), DBL2NUM(p.y), DBL2NUM(p.z));
}

static VALUE transformation_identity_p(VALUE self)
{
	bool result = false;
	SUTransformationIsIdentity(TR(self), &result);
	return BOOL2RB(result);
}

static VALUE transformation_inverse(VALUE self)
{
	struct SUTransformation inverse;
	if (SUTransformationGetInverse(TR(self), &inverse) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "transformation is not invertible");
	return su_transformation_new(inverse);
}

static VALUE transformation_invert_bang(VALUE self)
{
	*TR(self) = *TR(transformation_inverse(self));
	return self;
}

static VALUE transformation_origin(VALUE self)
{
	struct SUPoint3D origin;
	SUTransformationGetOrigin(TR(self), &origin);
	return su_point3d_new(origin);
}

#define DEFINE_AXIS(axis, SUFn) \
	static VALUE transformation_##axis(VALUE self) \
	{ \
		struct SUVector3D v; \
		SUFn(TR(self), &v); \
		return su_vector3d_new(v); \
	}
DEFINE_AXIS(xaxis, SUTransformationGetXAxis)
DEFINE_AXIS(yaxis, SUTransformationGetYAxis)
DEFINE_AXIS(zaxis, SUTransformationGetZAxis)

static VALUE transformation_to_a(VALUE self)
{
	VALUE ary = rb_ary_new_capa(16);
	for (int i = 0; i < 16; i++)
		rb_ary_push(ary, DBL2NUM(TR(self)->values[i]));
	return ary;
}

void Init_transformation(void)
{
	rb_define_alloc_func(cTransformation, transformation_alloc);
	rb_define_singleton_method(cTransformation, "axes", transformation_s_axes, -1);
	rb_define_singleton_method(cTransformation, "interpolate", transformation_s_interpolate, 3);
	rb_define_singleton_method(cTransformation, "rotation", transformation_s_rotation, 3);
	rb_define_singleton_method(cTransformation, "scaling", transformation_s_scaling, -1);
	rb_define_singleton_method(cTransformation, "translation", transformation_s_translation, 1);
	rb_define_method(cTransformation, "initialize", transformation_initialize, -1);
	rb_define_method(cTransformation, "initialize_copy", transformation_initialize_copy, 1);
	rb_define_method(cTransformation, "set!", transformation_set_bang, 1);
	rb_define_method(cTransformation, "*", transformation_mul, 1);
	rb_define_method(cTransformation, "identity?", transformation_identity_p, 0);
	rb_define_method(cTransformation, "inverse", transformation_inverse, 0);
	rb_define_method(cTransformation, "invert!", transformation_invert_bang, 0);
	rb_define_method(cTransformation, "origin", transformation_origin, 0);
	rb_define_method(cTransformation, "xaxis", transformation_xaxis, 0);
	rb_define_method(cTransformation, "yaxis", transformation_yaxis, 0);
	rb_define_method(cTransformation, "zaxis", transformation_zaxis, 0);
	rb_define_method(cTransformation, "to_a", transformation_to_a, 0);
}
