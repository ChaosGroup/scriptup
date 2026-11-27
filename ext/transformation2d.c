#include <scriptup.h>

#define TR(obj) ((struct SUTransformation2D*)DATA_PTR(obj))

static const struct SUTransformation2D identity = {1, 0, 0, 1, 0, 0};

static VALUE transformation2d_alloc(VALUE klass)
{
	struct SUTransformation2D* t;
	VALUE obj = Data_Make_Struct(klass, struct SUTransformation2D, 0, RUBY_DEFAULT_FREE, t);
	*t = identity;
	return obj;
}

static VALUE transformation2d_new(struct SUTransformation2D value)
{
	VALUE obj = transformation2d_alloc(cTransformation2d);
	*TR(obj) = value;
	return obj;
}

struct SUTransformation2D su_to_transformation2d(VALUE obj)
{
	return *(struct SUTransformation2D*)su_unwrap(obj, cTransformation2d);
}

static struct SUTransformation2D transformation2d_from(VALUE arg)
{
	if (rb_obj_is_kind_of(arg, cTransformation2d))
		return *TR(arg);
	if (!RB_TYPE_P(arg, T_ARRAY) || RARRAY_LEN(arg) != 6)
		rb_raise(rb_eArgError, "expected a Transformation2d or an array of 6 numbers");
	return (struct SUTransformation2D){NUM2DBL(RARRAY_AREF(arg, 0)), NUM2DBL(RARRAY_AREF(arg, 1)),
		NUM2DBL(RARRAY_AREF(arg, 2)), NUM2DBL(RARRAY_AREF(arg, 3)), NUM2DBL(RARRAY_AREF(arg, 4)),
		NUM2DBL(RARRAY_AREF(arg, 5))};
}

// translate(point) * t * translate(-point)
static struct SUTransformation2D about_point(struct SUTransformation2D t, struct SUPoint2D point)
{
	struct SUTransformation2D to = identity;
	struct SUTransformation2D back = identity;
	struct SUVector2D v = {point.x, point.y};
	SUTransformation2DTranslation(&to, &v);
	struct SUVector2D r = {-point.x, -point.y};
	SUTransformation2DTranslation(&back, &r);
	struct SUTransformation2D tmp;
	struct SUTransformation2D result;
	SUTransformation2DMultiply(&t, &back, &tmp);
	SUTransformation2DMultiply(&to, &tmp, &result);
	return result;
}

static VALUE transformation2d_initialize(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 0, 1);
	if (argc == 1)
		*TR(self) = transformation2d_from(argv[0]);
	return self;
}

static VALUE transformation2d_initialize_copy(VALUE self, VALUE other)
{
	*TR(self) = *TR(other);
	return self;
}

static VALUE transformation2d_set_bang(VALUE self, VALUE arg)
{
	*TR(self) = transformation2d_from(arg);
	return self;
}

static VALUE transformation2d_s_rotation(VALUE klass, VALUE point, VALUE angle)
{
	struct SUPoint2D origin = {0, 0};
	struct SUTransformation2D t;
	SUTransformation2DRotation(&t, &origin, NUM2DBL(angle));
	return transformation2d_new(about_point(t, su_to_point2d(point)));
}

static VALUE transformation2d_s_scaling(int argc, VALUE* argv, VALUE klass)
{
	rb_check_arity(argc, 1, 3);
	bool has_point = rb_obj_is_kind_of(argv[0], cPoint2d) || RB_TYPE_P(argv[0], T_ARRAY);
	int scales = has_point ? argc - 1 : argc;
	const VALUE* s = has_point ? argv + 1 : argv;
	struct SUTransformation2D t = identity;
	if (scales == 1)
		SUTransformation2DScale(&t, NUM2DBL(s[0]));
	else if (scales == 2)
		SUTransformation2DNonUniformScale(&t, NUM2DBL(s[0]), NUM2DBL(s[1]));
	else
		rb_raise(rb_eArgError, "expected 1 or 2 scale values");
	return transformation2d_new(has_point ? about_point(t, su_to_point2d(argv[0])) : t);
}

static VALUE transformation2d_s_translation(VALUE klass, VALUE vector)
{
	struct SUVector2D v;
	if (rb_obj_is_kind_of(vector, cPoint2d))
	{
		struct SUPoint2D p = su_to_point2d(vector);
		v = (struct SUVector2D){p.x, p.y};
	}
	else
		v = su_to_vector2d(vector);
	struct SUTransformation2D t = identity;
	SUTransformation2DTranslation(&t, &v);
	return transformation2d_new(t);
}

static VALUE transformation2d_mul(VALUE self, VALUE other)
{
	struct SUTransformation2D* t = TR(self);
	if (rb_obj_is_kind_of(other, cTransformation2d))
	{
		struct SUTransformation2D result;
		SUTransformation2DMultiply(t, TR(other), &result);
		return transformation2d_new(result);
	}
	if (rb_obj_is_kind_of(other, cVector2d))
	{
		struct SUVector2D v = su_to_vector2d(other);
		SUVector2DTransform(t, &v);
		return su_vector2d_new(v);
	}
	struct SUPoint2D p = su_to_point2d(other);
	SUPoint2DTransform(t, &p);
	if (rb_obj_is_kind_of(other, cPoint2d))
		return su_point2d_new(p);
	return rb_ary_new_from_args(2, DBL2NUM(p.x), DBL2NUM(p.y));
}

static VALUE transformation2d_eq(VALUE self, VALUE other)
{
	if (!rb_obj_is_kind_of(other, cTransformation2d))
		return Qfalse;
	struct SUTransformation2D* a = TR(self);
	struct SUTransformation2D* b = TR(other);
	const double tolerance = 1.0e-9;
	return BOOL2RB(fabs(a->m11 - b->m11) < tolerance && fabs(a->m12 - b->m12) < tolerance &&
		fabs(a->m21 - b->m21) < tolerance && fabs(a->m22 - b->m22) < tolerance &&
		fabs(a->tx - b->tx) < tolerance && fabs(a->ty - b->ty) < tolerance);
}

static VALUE transformation2d_identity_p(VALUE self)
{
	bool result = false;
	SUTransformation2DIsIdentity(TR(self), &result);
	return BOOL2RB(result);
}

static VALUE transformation2d_inverse(VALUE self)
{
	struct SUTransformation2D inverse;
	if (SUTransformation2DGetInverse(TR(self), &inverse) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "transformation is not invertible");
	return transformation2d_new(inverse);
}

static VALUE transformation2d_invert_bang(VALUE self)
{
	*TR(self) = *TR(transformation2d_inverse(self));
	return self;
}

static VALUE transformation2d_to_a(VALUE self)
{
	struct SUTransformation2D* t = TR(self);
	return rb_ary_new_from_args(6, DBL2NUM(t->m11), DBL2NUM(t->m12), DBL2NUM(t->m21), DBL2NUM(t->m22),
		DBL2NUM(t->tx), DBL2NUM(t->ty));
}

void Init_transformation2d(void)
{
	rb_define_alloc_func(cTransformation2d, transformation2d_alloc);
	rb_define_singleton_method(cTransformation2d, "rotation", transformation2d_s_rotation, 2);
	rb_define_singleton_method(cTransformation2d, "scaling", transformation2d_s_scaling, -1);
	rb_define_singleton_method(cTransformation2d, "translation", transformation2d_s_translation, 1);
	rb_define_method(cTransformation2d, "initialize", transformation2d_initialize, -1);
	rb_define_method(cTransformation2d, "initialize_copy", transformation2d_initialize_copy, 1);
	rb_define_method(cTransformation2d, "set!", transformation2d_set_bang, 1);
	rb_define_method(cTransformation2d, "*", transformation2d_mul, 1);
	rb_define_method(cTransformation2d, "==", transformation2d_eq, 1);
	rb_define_method(cTransformation2d, "identity?", transformation2d_identity_p, 0);
	rb_define_method(cTransformation2d, "inverse", transformation2d_inverse, 0);
	rb_define_method(cTransformation2d, "invert!", transformation2d_invert_bang, 0);
	rb_define_method(cTransformation2d, "to_a", transformation2d_to_a, 0);
}
