#include <scriptup.h>

#define VEC(obj) ((struct SUVector3D*)DATA_PTR(obj))

static VALUE vector3d_alloc(VALUE klass)
{
	struct SUVector3D* v;
	VALUE obj = Data_Make_Struct(klass, struct SUVector3D, 0, RUBY_DEFAULT_FREE, v);
	*v = (struct SUVector3D){0.0, 0.0, 0.0};
	return obj;
}

VALUE su_vector3d_new(struct SUVector3D value)
{
	VALUE obj = vector3d_alloc(cVector3d);
	*VEC(obj) = value;
	return obj;
}

// Accepts a Vector3d or a 2/3 element numeric array.
struct SUVector3D su_to_vector3d(VALUE obj)
{
	if (rb_obj_is_kind_of(obj, cVector3d))
		return *VEC(obj);
	double xyz[3];
	su_array_to_xyz(obj, xyz);
	return (struct SUVector3D){xyz[0], xyz[1], xyz[2]};
}

static VALUE vector3d_initialize(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 0, 3);
	if (argc == 1)
		*VEC(self) = su_to_vector3d(argv[0]);
	else if (argc == 2)
		rb_raise(rb_eArgError, "wrong number of arguments (given 2, expected 0, 1 or 3)");
	else if (argc == 3)
		*VEC(self) = (struct SUVector3D){NUM2DBL(argv[0]), NUM2DBL(argv[1]), NUM2DBL(argv[2])};
	return self;
}

static VALUE vector3d_initialize_copy(VALUE self, VALUE other)
{
	*VEC(self) = *VEC(other);
	return self;
}

static VALUE vector3d_set_bang(int argc, VALUE* argv, VALUE self)
{
	vector3d_initialize(argc, argv, self);
	return self;
}

static VALUE vector3d_linear_combination(int argc, VALUE* argv, VALUE klass)
{
	if (argc != 4 && argc != 6)
		rb_raise(rb_eArgError, "wrong number of arguments (given %d, expected 4 or 6)", argc);
	struct SUVector3D vectors[3];
	double weights[3];
	size_t count = (size_t)argc / 2;
	for (size_t i = 0; i < count; i++)
	{
		weights[i] = NUM2DBL(argv[i * 2]);
		vectors[i] = su_to_vector3d(argv[i * 2 + 1]);
	}
	struct SUVector3D result;
	SUVector3DLinearCombination(vectors, weights, count, &result);
	return su_vector3d_new(result);
}

#define DEFINE_COMPONENT(axis) \
	static VALUE vector3d_##axis(VALUE self) { return DBL2NUM(VEC(self)->axis); } \
	static VALUE vector3d_set_##axis(VALUE self, VALUE value) \
	{ \
		VEC(self)->axis = NUM2DBL(value); \
		return value; \
	}
DEFINE_COMPONENT(x)
DEFINE_COMPONENT(y)
DEFINE_COMPONENT(z)

static double* vector3d_component(VALUE self, VALUE index)
{
	long i = NUM2LONG(index);
	if (i < 0 || i > 2)
		rb_raise(rb_eIndexError, "index %ld out of range [0, 2]", i);
	double* components[3] = {&VEC(self)->x, &VEC(self)->y, &VEC(self)->z};
	return components[i];
}

static VALUE vector3d_aref(VALUE self, VALUE index)
{
	return su_length_new(*vector3d_component(self, index));
}

static VALUE vector3d_aset(VALUE self, VALUE index, VALUE value)
{
	*vector3d_component(self, index) = NUM2DBL(value);
	return value;
}

static VALUE vector3d_to_a(VALUE self)
{
	struct SUVector3D* v = VEC(self);
	return rb_ary_new_from_args(3, DBL2NUM(v->x), DBL2NUM(v->y), DBL2NUM(v->z));
}

static VALUE vector3d_to_s(VALUE self)
{
	struct SUVector3D* v = VEC(self);
	return rb_sprintf("(%"PRIsVALUE", %"PRIsVALUE", %"PRIsVALUE")",
		su_format_number(v->x), su_format_number(v->y), su_format_number(v->z));
}

static VALUE vector3d_inspect(VALUE self)
{
	return rb_sprintf("Vector3d%"PRIsVALUE, vector3d_to_s(self));
}

static VALUE vector3d_plus(VALUE self, VALUE other)
{
	struct SUVector3D a = *VEC(self);
	struct SUVector3D b = su_to_vector3d(other);
	return su_vector3d_new((struct SUVector3D){a.x + b.x, a.y + b.y, a.z + b.z});
}

static VALUE vector3d_minus(VALUE self, VALUE other)
{
	struct SUVector3D a = *VEC(self);
	struct SUVector3D b = su_to_vector3d(other);
	return su_vector3d_new((struct SUVector3D){a.x - b.x, a.y - b.y, a.z - b.z});
}

static VALUE vector3d_cross(VALUE self, VALUE other)
{
	struct SUVector3D b = su_to_vector3d(other);
	struct SUVector3D result;
	SUVector3DCross(VEC(self), &b, &result);
	return su_vector3d_new(result);
}

static VALUE vector3d_dot(VALUE self, VALUE other)
{
	struct SUVector3D b = su_to_vector3d(other);
	double result = 0.0;
	SUVector3DDot(VEC(self), &b, &result);
	return DBL2NUM(result);
}

static VALUE vector3d_angle_between(VALUE self, VALUE other)
{
	struct SUVector3D b = su_to_vector3d(other);
	double angle = 0.0;
	SUVector3DAngleBetween(VEC(self), &b, &angle);
	return DBL2NUM(angle);
}

#define DEFINE_VECTOR_PREDICATE(fn, SUFn) \
	static VALUE fn(VALUE self, VALUE other) \
	{ \
		struct SUVector3D b = su_to_vector3d(other); \
		bool result = false; \
		SUFn(VEC(self), &b, &result); \
		return BOOL2RB(result); \
	}
DEFINE_VECTOR_PREDICATE(vector3d_parallel_p, SUVector3DIsParallelTo)
DEFINE_VECTOR_PREDICATE(vector3d_perpendicular_p, SUVector3DIsPerpendicularTo)
DEFINE_VECTOR_PREDICATE(vector3d_samedirection_p, SUVector3DIsSameDirectionAs)
DEFINE_VECTOR_PREDICATE(vector3d_lt, SUVector3DLessThan)

static VALUE vector3d_eq(VALUE self, VALUE other)
{
	if (!rb_obj_is_kind_of(other, cVector3d) && !RB_TYPE_P(other, T_ARRAY))
		return Qfalse;
	struct SUVector3D b = su_to_vector3d(other);
	bool equal = false;
	SUVector3DIsEqual(VEC(self), &b, &equal);
	return BOOL2RB(equal);
}

static VALUE vector3d_unitvector_p(VALUE self)
{
	bool unit = false;
	SUVector3DIsUnitVector(VEC(self), &unit);
	return BOOL2RB(unit);
}

static VALUE vector3d_valid_p(VALUE self)
{
	bool valid = false;
	SUVector3DIsValid(VEC(self), &valid);
	return BOOL2RB(valid);
}

static VALUE vector3d_length(VALUE self)
{
	double length = 0.0;
	SUVector3DGetLength(VEC(self), &length);
	return su_length_new(length);
}

static VALUE vector3d_set_length(VALUE self, VALUE length)
{
	if (SUVector3DSetLength(VEC(self), NUM2DBL(length)) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "Cannot set the length of a zero length vector");
	return length;
}

static VALUE vector3d_normalize_bang(VALUE self)
{
	SUVector3DNormalize(VEC(self));
	return self;
}

static VALUE vector3d_normalize(VALUE self)
{
	return vector3d_normalize_bang(su_vector3d_new(*VEC(self)));
}

static VALUE vector3d_reverse_bang(VALUE self)
{
	SUVector3DReverse(VEC(self));
	return self;
}

static VALUE vector3d_reverse(VALUE self)
{
	return vector3d_reverse_bang(su_vector3d_new(*VEC(self)));
}

static VALUE vector3d_axes(VALUE self)
{
	struct SUVector3D x_axis;
	struct SUVector3D y_axis;
	SUVector3DGetArbitraryAxes(VEC(self), &x_axis, &y_axis);
	struct SUVector3D z_axis = *VEC(self);
	SUVector3DNormalize(&z_axis);
	return rb_ary_new_from_args(3, su_vector3d_new(x_axis), su_vector3d_new(y_axis), su_vector3d_new(z_axis));
}

void Init_vector3d(void)
{
	rb_define_alloc_func(cVector3d, vector3d_alloc);
	rb_define_singleton_method(cVector3d, "linear_combination", vector3d_linear_combination, -1);
	rb_define_method(cVector3d, "initialize", vector3d_initialize, -1);
	rb_define_method(cVector3d, "initialize_copy", vector3d_initialize_copy, 1);
	rb_define_method(cVector3d, "set!", vector3d_set_bang, -1);
	rb_define_method(cVector3d, "x", vector3d_x, 0);
	rb_define_method(cVector3d, "x=", vector3d_set_x, 1);
	rb_define_method(cVector3d, "y", vector3d_y, 0);
	rb_define_method(cVector3d, "y=", vector3d_set_y, 1);
	rb_define_method(cVector3d, "z", vector3d_z, 0);
	rb_define_method(cVector3d, "z=", vector3d_set_z, 1);
	rb_define_method(cVector3d, "[]", vector3d_aref, 1);
	rb_define_method(cVector3d, "[]=", vector3d_aset, 2);
	rb_define_method(cVector3d, "to_a", vector3d_to_a, 0);
	rb_define_method(cVector3d, "to_s", vector3d_to_s, 0);
	rb_define_method(cVector3d, "inspect", vector3d_inspect, 0);
	rb_define_method(cVector3d, "+", vector3d_plus, 1);
	rb_define_method(cVector3d, "-", vector3d_minus, 1);
	rb_define_method(cVector3d, "*", vector3d_cross, 1);
	rb_define_method(cVector3d, "cross", vector3d_cross, 1);
	rb_define_method(cVector3d, "%", vector3d_dot, 1);
	rb_define_method(cVector3d, "dot", vector3d_dot, 1);
	rb_define_method(cVector3d, "<", vector3d_lt, 1);
	rb_define_method(cVector3d, "==", vector3d_eq, 1);
	rb_define_method(cVector3d, "angle_between", vector3d_angle_between, 1);
	rb_define_method(cVector3d, "parallel?", vector3d_parallel_p, 1);
	rb_define_method(cVector3d, "perpendicular?", vector3d_perpendicular_p, 1);
	rb_define_method(cVector3d, "samedirection?", vector3d_samedirection_p, 1);
	rb_define_method(cVector3d, "unitvector?", vector3d_unitvector_p, 0);
	rb_define_method(cVector3d, "valid?", vector3d_valid_p, 0);
	rb_define_method(cVector3d, "length", vector3d_length, 0);
	rb_define_method(cVector3d, "length=", vector3d_set_length, 1);
	rb_define_method(cVector3d, "normalize", vector3d_normalize, 0);
	rb_define_method(cVector3d, "normalize!", vector3d_normalize_bang, 0);
	rb_define_method(cVector3d, "reverse", vector3d_reverse, 0);
	rb_define_method(cVector3d, "reverse!", vector3d_reverse_bang, 0);
	rb_define_method(cVector3d, "axes", vector3d_axes, 0);
}
