#include <scriptup.h>

#define VEC(obj) ((struct SUVector2D*)DATA_PTR(obj))

static VALUE vector2d_alloc(VALUE klass)
{
	struct SUVector2D* v;
	VALUE obj = Data_Make_Struct(klass, struct SUVector2D, 0, RUBY_DEFAULT_FREE, v);
	*v = (struct SUVector2D){0.0, 0.0};
	return obj;
}

VALUE su_vector2d_new(struct SUVector2D value)
{
	VALUE obj = vector2d_alloc(cVector2d);
	*VEC(obj) = value;
	return obj;
}

struct SUVector2D su_to_vector2d(VALUE obj)
{
	if (rb_obj_is_kind_of(obj, cVector2d))
		return *VEC(obj);
	double xy[2];
	su_array_to_xy(obj, xy);
	return (struct SUVector2D){xy[0], xy[1]};
}

static VALUE vector2d_initialize(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 0, 2);
	if (argc == 1)
		*VEC(self) = su_to_vector2d(argv[0]);
	else if (argc == 2)
		*VEC(self) = (struct SUVector2D){NUM2DBL(argv[0]), NUM2DBL(argv[1])};
	return self;
}

static VALUE vector2d_initialize_copy(VALUE self, VALUE other)
{
	*VEC(self) = *VEC(other);
	return self;
}

static VALUE vector2d_set_bang(int argc, VALUE* argv, VALUE self)
{
	return vector2d_initialize(argc, argv, self);
}

#define DEFINE_COMPONENT(axis) \
	static VALUE vector2d_##axis(VALUE self) { return DBL2NUM(VEC(self)->axis); } \
	static VALUE vector2d_set_##axis(VALUE self, VALUE value) \
	{ \
		VEC(self)->axis = NUM2DBL(value); \
		return value; \
	}
DEFINE_COMPONENT(x)
DEFINE_COMPONENT(y)

static double* vector2d_component(VALUE self, VALUE index)
{
	long i = NUM2LONG(index);
	if (i < 0 || i > 1)
		rb_raise(rb_eIndexError, "index %ld out of range [0, 1]", i);
	return i == 0 ? &VEC(self)->x : &VEC(self)->y;
}

static VALUE vector2d_aref(VALUE self, VALUE index)
{
	return DBL2NUM(*vector2d_component(self, index));
}

static VALUE vector2d_aset(VALUE self, VALUE index, VALUE value)
{
	*vector2d_component(self, index) = NUM2DBL(value);
	return value;
}

static VALUE vector2d_to_a(VALUE self)
{
	return rb_ary_new_from_args(2, DBL2NUM(VEC(self)->x), DBL2NUM(VEC(self)->y));
}

static VALUE vector2d_to_s(VALUE self)
{
	return rb_sprintf("(%"PRIsVALUE", %"PRIsVALUE")", su_format_number(VEC(self)->x), su_format_number(VEC(self)->y));
}

static VALUE vector2d_inspect(VALUE self)
{
	return rb_sprintf("Vector2d%"PRIsVALUE, vector2d_to_s(self));
}

static VALUE vector2d_plus(VALUE self, VALUE other)
{
	struct SUVector2D b = su_to_vector2d(other);
	return su_vector2d_new((struct SUVector2D){VEC(self)->x + b.x, VEC(self)->y + b.y});
}

static VALUE vector2d_minus(VALUE self, VALUE other)
{
	struct SUVector2D b = su_to_vector2d(other);
	return su_vector2d_new((struct SUVector2D){VEC(self)->x - b.x, VEC(self)->y - b.y});
}

#define DEFINE_SCALAR_OP(fn, SUFn) \
	static VALUE fn(VALUE self, VALUE other) \
	{ \
		struct SUVector2D b = su_to_vector2d(other); \
		double result = 0.0; \
		SUFn(VEC(self), &b, &result); \
		return DBL2NUM(result); \
	}
DEFINE_SCALAR_OP(vector2d_dot, SUVector2DDot)
DEFINE_SCALAR_OP(vector2d_cross, SUVector2DCross)
DEFINE_SCALAR_OP(vector2d_angle_between, SUVector2DAngleBetween)

#define DEFINE_PREDICATE(fn, SUFn) \
	static VALUE fn(VALUE self, VALUE other) \
	{ \
		struct SUVector2D b = su_to_vector2d(other); \
		bool result = false; \
		SUFn(VEC(self), &b, &result); \
		return BOOL2RB(result); \
	}
DEFINE_PREDICATE(vector2d_parallel_p, SUVector2DIsParallelTo)
DEFINE_PREDICATE(vector2d_perpendicular_p, SUVector2DIsPerpendicularTo)
DEFINE_PREDICATE(vector2d_same_direction_p, SUVector2DIsSameDirectionAs)

static VALUE vector2d_eq(VALUE self, VALUE other)
{
	if (!rb_obj_is_kind_of(other, cVector2d) && !RB_TYPE_P(other, T_ARRAY))
		return Qfalse;
	struct SUVector2D b = su_to_vector2d(other);
	bool equal = false;
	SUVector2DIsEqual(VEC(self), &b, &equal);
	return BOOL2RB(equal);
}

static VALUE vector2d_unit_vector_p(VALUE self)
{
	bool unit = false;
	SUVector2DIsUnitVector(VEC(self), &unit);
	return BOOL2RB(unit);
}

static VALUE vector2d_valid_p(VALUE self)
{
	bool valid = false;
	SUVector2DIsValid(VEC(self), &valid);
	return BOOL2RB(valid);
}

static VALUE vector2d_length(VALUE self)
{
	double length = 0.0;
	SUVector2DGetLength(VEC(self), &length);
	return su_length_new(length);
}

static VALUE vector2d_set_length(VALUE self, VALUE length)
{
	if (SUVector2DSetLength(VEC(self), NUM2DBL(length)) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "Cannot set the length of a zero length vector");
	return length;
}

static VALUE vector2d_normalize_bang(VALUE self)
{
	SUVector2DNormalize(VEC(self));
	return self;
}

static VALUE vector2d_normalize(VALUE self)
{
	return vector2d_normalize_bang(su_vector2d_new(*VEC(self)));
}

static VALUE vector2d_reverse_bang(VALUE self)
{
	SUVector2DReverse(VEC(self));
	return self;
}

static VALUE vector2d_reverse(VALUE self)
{
	return vector2d_reverse_bang(su_vector2d_new(*VEC(self)));
}

void Init_vector2d(void)
{
	rb_define_alloc_func(cVector2d, vector2d_alloc);
	rb_define_method(cVector2d, "initialize", vector2d_initialize, -1);
	rb_define_method(cVector2d, "initialize_copy", vector2d_initialize_copy, 1);
	rb_define_method(cVector2d, "set!", vector2d_set_bang, -1);
	rb_define_method(cVector2d, "x", vector2d_x, 0);
	rb_define_method(cVector2d, "x=", vector2d_set_x, 1);
	rb_define_method(cVector2d, "y", vector2d_y, 0);
	rb_define_method(cVector2d, "y=", vector2d_set_y, 1);
	rb_define_method(cVector2d, "[]", vector2d_aref, 1);
	rb_define_method(cVector2d, "[]=", vector2d_aset, 2);
	rb_define_method(cVector2d, "to_a", vector2d_to_a, 0);
	rb_define_method(cVector2d, "to_s", vector2d_to_s, 0);
	rb_define_method(cVector2d, "inspect", vector2d_inspect, 0);
	rb_define_method(cVector2d, "+", vector2d_plus, 1);
	rb_define_method(cVector2d, "-", vector2d_minus, 1);
	rb_define_method(cVector2d, "%", vector2d_dot, 1);
	rb_define_method(cVector2d, "dot", vector2d_dot, 1);
	rb_define_method(cVector2d, "*", vector2d_cross, 1);
	rb_define_method(cVector2d, "cross", vector2d_cross, 1);
	rb_define_method(cVector2d, "angle_between", vector2d_angle_between, 1);
	rb_define_method(cVector2d, "==", vector2d_eq, 1);
	rb_define_method(cVector2d, "parallel?", vector2d_parallel_p, 1);
	rb_define_method(cVector2d, "perpendicular?", vector2d_perpendicular_p, 1);
	rb_define_method(cVector2d, "same_direction?", vector2d_same_direction_p, 1);
	rb_define_method(cVector2d, "unit_vector?", vector2d_unit_vector_p, 0);
	rb_define_method(cVector2d, "valid?", vector2d_valid_p, 0);
	rb_define_method(cVector2d, "length", vector2d_length, 0);
	rb_define_method(cVector2d, "length=", vector2d_set_length, 1);
	rb_define_method(cVector2d, "normalize", vector2d_normalize, 0);
	rb_define_method(cVector2d, "normalize!", vector2d_normalize_bang, 0);
	rb_define_method(cVector2d, "reverse", vector2d_reverse, 0);
	rb_define_method(cVector2d, "reverse!", vector2d_reverse_bang, 0);
}
