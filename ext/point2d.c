#include <scriptup.h>

#define PT(obj) ((struct SUPoint2D*)DATA_PTR(obj))

static VALUE point2d_alloc(VALUE klass)
{
	struct SUPoint2D* p;
	VALUE obj = Data_Make_Struct(klass, struct SUPoint2D, 0, RUBY_DEFAULT_FREE, p);
	*p = (struct SUPoint2D){0.0, 0.0};
	return obj;
}

VALUE su_point2d_new(struct SUPoint2D value)
{
	VALUE obj = point2d_alloc(cPoint2d);
	*PT(obj) = value;
	return obj;
}

struct SUPoint2D su_to_point2d(VALUE obj)
{
	if (rb_obj_is_kind_of(obj, cPoint2d))
		return *PT(obj);
	double xy[2];
	su_array_to_xy(obj, xy);
	return (struct SUPoint2D){xy[0], xy[1]};
}

static VALUE point2d_initialize(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 0, 2);
	if (argc == 1)
		*PT(self) = su_to_point2d(argv[0]);
	else if (argc == 2)
		*PT(self) = (struct SUPoint2D){NUM2DBL(argv[0]), NUM2DBL(argv[1])};
	return self;
}

static VALUE point2d_initialize_copy(VALUE self, VALUE other)
{
	*PT(self) = *PT(other);
	return self;
}

static VALUE point2d_set_bang(int argc, VALUE* argv, VALUE self)
{
	return point2d_initialize(argc, argv, self);
}

#define DEFINE_COMPONENT(axis) \
	static VALUE point2d_##axis(VALUE self) { return su_length_new(PT(self)->axis); } \
	static VALUE point2d_set_##axis(VALUE self, VALUE value) \
	{ \
		PT(self)->axis = NUM2DBL(value); \
		return value; \
	}
DEFINE_COMPONENT(x)
DEFINE_COMPONENT(y)

static double* point2d_component(VALUE self, VALUE index)
{
	long i = NUM2LONG(index);
	if (i < 0 || i > 1)
		rb_raise(rb_eIndexError, "index %ld out of range [0, 1]", i);
	return i == 0 ? &PT(self)->x : &PT(self)->y;
}

static VALUE point2d_aref(VALUE self, VALUE index)
{
	return su_length_new(*point2d_component(self, index));
}

static VALUE point2d_aset(VALUE self, VALUE index, VALUE value)
{
	*point2d_component(self, index) = NUM2DBL(value);
	return value;
}

static VALUE point2d_to_a(VALUE self)
{
	return rb_ary_new_from_args(2, su_length_new(PT(self)->x), su_length_new(PT(self)->y));
}

static VALUE point2d_to_s(VALUE self)
{
	return rb_sprintf("(%"PRIsVALUE", %"PRIsVALUE")",
		rb_obj_as_string(su_length_new(PT(self)->x)), rb_obj_as_string(su_length_new(PT(self)->y)));
}

static VALUE point2d_inspect(VALUE self)
{
	return rb_sprintf("Point2d(%"PRIsVALUE", %"PRIsVALUE")", su_format_number(PT(self)->x), su_format_number(PT(self)->y));
}

static VALUE point2d_plus(VALUE self, VALUE vector)
{
	struct SUVector2D v = su_to_vector2d(vector);
	struct SUPoint2D result;
	SUPoint2DOffset(PT(self), &v, &result);
	return su_point2d_new(result);
}

static VALUE point2d_minus(VALUE self, VALUE other)
{
	if (rb_obj_is_kind_of(other, cVector2d))
	{
		struct SUVector2D v = su_to_vector2d(other);
		return su_point2d_new((struct SUPoint2D){PT(self)->x - v.x, PT(self)->y - v.y});
	}
	struct SUPoint2D q = su_to_point2d(other);
	return su_vector2d_new((struct SUVector2D){PT(self)->x - q.x, PT(self)->y - q.y});
}

static VALUE point2d_eq(VALUE self, VALUE other)
{
	if (!rb_obj_is_kind_of(other, cPoint2d) && !RB_TYPE_P(other, T_ARRAY))
		return Qfalse;
	struct SUPoint2D q = su_to_point2d(other);
	bool equal = false;
	SUPoint2DGetEqual(PT(self), &q, &equal);
	return BOOL2RB(equal);
}

static VALUE point2d_distance(VALUE self, VALUE other)
{
	struct SUPoint2D q = su_to_point2d(other);
	double distance = 0.0;
	SUPoint2DDistanceToSUPoint2D(PT(self), &q, &distance);
	return su_length_new(distance);
}

static VALUE point2d_vector_to(VALUE self, VALUE other)
{
	struct SUPoint2D q = su_to_point2d(other);
	struct SUVector2D v;
	SUVector2DCreate(&v, PT(self), &q);
	return su_vector2d_new(v);
}

static VALUE point2d_offset_bang(int argc, VALUE* argv, VALUE self)
{
	VALUE vector;
	VALUE distance;
	rb_scan_args(argc, argv, "11", &vector, &distance);
	struct SUVector2D v = su_to_vector2d(vector);
	if (!NIL_P(distance) && SUVector2DSetLength(&v, NUM2DBL(distance)) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "Cannot offset by a zero length vector");
	struct SUPoint2D result;
	SUPoint2DOffset(PT(self), &v, &result);
	*PT(self) = result;
	return self;
}

static VALUE point2d_offset(int argc, VALUE* argv, VALUE self)
{
	return point2d_offset_bang(argc, argv, su_point2d_new(*PT(self)));
}

static VALUE point2d_transform_bang(VALUE self, VALUE transformation)
{
	struct SUTransformation2D t = su_to_transformation2d(transformation);
	SUPoint2DTransform(&t, PT(self));
	return self;
}

static VALUE point2d_transform(VALUE self, VALUE transformation)
{
	return point2d_transform_bang(su_point2d_new(*PT(self)), transformation);
}

void Init_point2d(void)
{
	rb_define_alloc_func(cPoint2d, point2d_alloc);
	rb_define_method(cPoint2d, "transform", point2d_transform, 1);
	rb_define_method(cPoint2d, "transform!", point2d_transform_bang, 1);
	rb_define_method(cPoint2d, "initialize", point2d_initialize, -1);
	rb_define_method(cPoint2d, "initialize_copy", point2d_initialize_copy, 1);
	rb_define_method(cPoint2d, "set!", point2d_set_bang, -1);
	rb_define_method(cPoint2d, "x", point2d_x, 0);
	rb_define_method(cPoint2d, "x=", point2d_set_x, 1);
	rb_define_method(cPoint2d, "y", point2d_y, 0);
	rb_define_method(cPoint2d, "y=", point2d_set_y, 1);
	rb_define_method(cPoint2d, "[]", point2d_aref, 1);
	rb_define_method(cPoint2d, "[]=", point2d_aset, 2);
	rb_define_method(cPoint2d, "to_a", point2d_to_a, 0);
	rb_define_method(cPoint2d, "to_s", point2d_to_s, 0);
	rb_define_method(cPoint2d, "inspect", point2d_inspect, 0);
	rb_define_method(cPoint2d, "+", point2d_plus, 1);
	rb_define_method(cPoint2d, "-", point2d_minus, 1);
	rb_define_method(cPoint2d, "==", point2d_eq, 1);
	rb_define_method(cPoint2d, "distance", point2d_distance, 1);
	rb_define_method(cPoint2d, "vector_to", point2d_vector_to, 1);
	rb_define_method(cPoint2d, "offset", point2d_offset, -1);
	rb_define_method(cPoint2d, "offset!", point2d_offset_bang, -1);
}
