#include <scriptup.h>

#define PT(obj) ((struct SUPoint3D*)DATA_PTR(obj))

static VALUE point3d_alloc(VALUE klass)
{
	struct SUPoint3D* p;
	VALUE obj = Data_Make_Struct(klass, struct SUPoint3D, 0, RUBY_DEFAULT_FREE, p);
	*p = (struct SUPoint3D){0.0, 0.0, 0.0};
	return obj;
}

VALUE su_point3d_new(struct SUPoint3D value)
{
	VALUE obj = point3d_alloc(cPoint3d);
	*PT(obj) = value;
	return obj;
}

// Accepts a Point3d or a 2/3 element numeric array.
struct SUPoint3D su_to_point3d(VALUE obj)
{
	if (rb_obj_is_kind_of(obj, cPoint3d))
		return *PT(obj);
	double xyz[3];
	su_array_to_xyz(obj, xyz);
	return (struct SUPoint3D){xyz[0], xyz[1], xyz[2]};
}

// Geom::point3d_like: also accepts a Vertex.
struct SUPoint3D su_to_point3d_like(VALUE obj)
{
	if (rb_obj_is_kind_of(obj, cVertex))
	{
		struct SUPoint3D p = {0, 0, 0};
		SUVertexGetPosition(SU_REF(SUVertexRef, obj), &p);
		return p;
	}
	return su_to_point3d(obj);
}

// A line is [point, vector] or [point, point].
struct SURay3D su_to_ray3d(VALUE line)
{
	if (!RB_TYPE_P(line, T_ARRAY) || RARRAY_LEN(line) != 2)
		rb_raise(rb_eArgError, "a line must be [point, vector] or [point, point]");
	struct SURay3D ray;
	ray.point = su_to_point3d(RARRAY_AREF(line, 0));
	VALUE second = RARRAY_AREF(line, 1);
	if (rb_obj_is_kind_of(second, cPoint3d))
	{
		struct SUPoint3D to = *PT(second);
		SUVector3DCreate(&ray.normal, &ray.point, &to);
	}
	else
		ray.normal = su_to_vector3d(second);
	SUVector3DNormalize(&ray.normal);
	return ray;
}

// A plane is [point, normal], [a, b, c, d] or [point, point, point].
struct SUPlane3D su_to_plane3d(VALUE plane)
{
	if (!RB_TYPE_P(plane, T_ARRAY))
		rb_raise(rb_eArgError, "a plane must be [point, normal] or [a, b, c, d]");
	struct SUPlane3D result = {0.0, 0.0, 1.0, 0.0};
	enum SUResult status = SU_ERROR_INVALID_INPUT;
	long length = RARRAY_LEN(plane);
	if (length == 2)
	{
		struct SUPoint3D point = su_to_point3d(RARRAY_AREF(plane, 0));
		struct SUVector3D normal = su_to_vector3d(RARRAY_AREF(plane, 1));
		status = SUPlane3DSetWithPointAndNormal(&result, &point, &normal);
	}
	else if (length == 3)
	{
		struct SUPoint3D a = su_to_point3d(RARRAY_AREF(plane, 0));
		struct SUPoint3D b = su_to_point3d(RARRAY_AREF(plane, 1));
		struct SUPoint3D c = su_to_point3d(RARRAY_AREF(plane, 2));
		status = SUPlane3DSetWithPoints(&result, &a, &b, &c);
	}
	else if (length == 4)
		status = SUPlane3DSetWithCoefficients(&result, NUM2DBL(RARRAY_AREF(plane, 0)), NUM2DBL(RARRAY_AREF(plane, 1)),
			NUM2DBL(RARRAY_AREF(plane, 2)), NUM2DBL(RARRAY_AREF(plane, 3)));
	if (status != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "invalid plane");
	return result;
}

static VALUE point3d_initialize(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 0, 3);
	if (argc == 1)
		*PT(self) = su_to_point3d(argv[0]);
	else if (argc >= 2)
		*PT(self) = (struct SUPoint3D){NUM2DBL(argv[0]), NUM2DBL(argv[1]), argc == 3 ? NUM2DBL(argv[2]) : 0.0};
	return self;
}

static VALUE point3d_initialize_copy(VALUE self, VALUE other)
{
	*PT(self) = *PT(other);
	return self;
}

static VALUE point3d_set_bang(int argc, VALUE* argv, VALUE self)
{
	return point3d_initialize(argc, argv, self);
}

static VALUE point3d_linear_combination(VALUE klass, VALUE w1, VALUE p1, VALUE w2, VALUE p2)
{
	double a = NUM2DBL(w1);
	double b = NUM2DBL(w2);
	struct SUPoint3D p = su_to_point3d(p1);
	struct SUPoint3D q = su_to_point3d(p2);
	return su_point3d_new((struct SUPoint3D){a * p.x + b * q.x, a * p.y + b * q.y, a * p.z + b * q.z});
}

#define DEFINE_COMPONENT(axis) \
	static VALUE point3d_##axis(VALUE self) { return su_length_new(PT(self)->axis); } \
	static VALUE point3d_set_##axis(VALUE self, VALUE value) \
	{ \
		PT(self)->axis = NUM2DBL(value); \
		return value; \
	}
DEFINE_COMPONENT(x)
DEFINE_COMPONENT(y)
DEFINE_COMPONENT(z)

static double* point3d_component(VALUE self, VALUE index)
{
	long i = NUM2LONG(index);
	if (i < 0 || i > 2)
		rb_raise(rb_eIndexError, "index %ld out of range [0, 2]", i);
	double* components[3] = {&PT(self)->x, &PT(self)->y, &PT(self)->z};
	return components[i];
}

static VALUE point3d_aref(VALUE self, VALUE index)
{
	return su_length_new(*point3d_component(self, index));
}

static VALUE point3d_aset(VALUE self, VALUE index, VALUE value)
{
	*point3d_component(self, index) = NUM2DBL(value);
	return value;
}

static VALUE point3d_to_a(VALUE self)
{
	struct SUPoint3D* p = PT(self);
	return rb_ary_new_from_args(3, su_length_new(p->x), su_length_new(p->y), su_length_new(p->z));
}

static VALUE point3d_to_s(VALUE self)
{
	VALUE parts = rb_ary_new_capa(3);
	VALUE values = point3d_to_a(self);
	for (long i = 0; i < 3; i++)
		rb_ary_push(parts, rb_obj_as_string(RARRAY_AREF(values, i)));
	return rb_sprintf("(%"PRIsVALUE")", rb_ary_join(parts, rb_str_new_cstr(", ")));
}

static VALUE point3d_inspect(VALUE self)
{
	struct SUPoint3D* p = PT(self);
	return rb_sprintf("Point3d(%"PRIsVALUE", %"PRIsVALUE", %"PRIsVALUE")",
		su_format_number(p->x), su_format_number(p->y), su_format_number(p->z));
}

static VALUE point3d_plus(VALUE self, VALUE vector)
{
	struct SUVector3D v = su_to_vector3d(vector);
	struct SUPoint3D result;
	SUPoint3DOffset(PT(self), &v, &result);
	return su_point3d_new(result);
}

static VALUE point3d_minus(VALUE self, VALUE other)
{
	struct SUPoint3D* p = PT(self);
	if (rb_obj_is_kind_of(other, cVector3d))
	{
		struct SUVector3D v = su_to_vector3d(other);
		return su_point3d_new((struct SUPoint3D){p->x - v.x, p->y - v.y, p->z - v.z});
	}
	struct SUPoint3D q = su_to_point3d(other);
	return su_vector3d_new((struct SUVector3D){p->x - q.x, p->y - q.y, p->z - q.z});
}

static VALUE point3d_lt(VALUE self, VALUE other)
{
	struct SUPoint3D q = su_to_point3d(other);
	bool less = false;
	SUPoint3DLessThan(PT(self), &q, &less);
	return BOOL2RB(less);
}

static VALUE point3d_eq(VALUE self, VALUE other)
{
	if (!rb_obj_is_kind_of(other, cPoint3d) && !RB_TYPE_P(other, T_ARRAY))
		return Qfalse;
	struct SUPoint3D q = su_to_point3d(other);
	bool equal = false;
	SUPoint3DGetEqual(PT(self), &q, &equal);
	return BOOL2RB(equal);
}

static VALUE point3d_distance(VALUE self, VALUE other)
{
	struct SUPoint3D q = su_to_point3d(other);
	double distance = 0.0;
	SUPoint3DDistanceToSUPoint3D(PT(self), &q, &distance);
	return su_length_new(distance);
}

static VALUE point3d_vector_to(VALUE self, VALUE other)
{
	struct SUPoint3D q = su_to_point3d(other);
	struct SUVector3D v;
	SUVector3DCreate(&v, PT(self), &q);
	return su_vector3d_new(v);
}

static VALUE point3d_offset_bang(int argc, VALUE* argv, VALUE self)
{
	VALUE vector;
	VALUE length;
	rb_scan_args(argc, argv, "11", &vector, &length);
	struct SUVector3D v = su_to_vector3d(vector);
	if (!NIL_P(length) && SUVector3DSetLength(&v, NUM2DBL(length)) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "Cannot offset by a zero length vector");
	struct SUPoint3D result;
	SUPoint3DOffset(PT(self), &v, &result);
	*PT(self) = result;
	return self;
}

static VALUE point3d_offset(int argc, VALUE* argv, VALUE self)
{
	return point3d_offset_bang(argc, argv, su_point3d_new(*PT(self)));
}

static VALUE point3d_distance_to_line(VALUE self, VALUE line)
{
	struct SURay3D ray = su_to_ray3d(line);
	double distance = 0.0;
	SURay3DDistanceTo(&ray, PT(self), &distance);
	return DBL2NUM(distance);
}

static VALUE point3d_on_line_p(VALUE self, VALUE line)
{
	struct SURay3D ray = su_to_ray3d(line);
	bool on = false;
	SURay3DIsOn(&ray, PT(self), &on);
	return BOOL2RB(on);
}

static VALUE point3d_project_to_line(VALUE self, VALUE line)
{
	struct SURay3D ray = su_to_ray3d(line);
	struct SUPoint3D result;
	SURay3DProjectTo(&ray, PT(self), &result);
	return su_point3d_new(result);
}

static VALUE point3d_distance_to_plane(VALUE self, VALUE plane)
{
	struct SUPlane3D p = su_to_plane3d(plane);
	double distance = 0.0;
	SUPlane3DDistanceTo(&p, PT(self), &distance);
	return DBL2NUM(distance);
}

static VALUE point3d_on_plane_p(VALUE self, VALUE plane)
{
	struct SUPlane3D p = su_to_plane3d(plane);
	bool on = false;
	SUPlane3DIsOn(&p, PT(self), &on);
	return BOOL2RB(on);
}

static VALUE point3d_project_to_plane(VALUE self, VALUE plane)
{
	struct SUPlane3D p = su_to_plane3d(plane);
	struct SUPoint3D result;
	SUPlane3DProjectTo(&p, PT(self), &result);
	return su_point3d_new(result);
}

static VALUE point3d_transform_bang(VALUE self, VALUE transformation)
{
	struct SUTransformation t = su_to_transformation(transformation);
	SUPoint3DTransform(&t, PT(self));
	return self;
}

static VALUE point3d_transform(VALUE self, VALUE transformation)
{
	return point3d_transform_bang(su_point3d_new(*PT(self)), transformation);
}

void Init_point3d(void)
{
	rb_define_alloc_func(cPoint3d, point3d_alloc);
	rb_define_method(cPoint3d, "transform", point3d_transform, 1);
	rb_define_method(cPoint3d, "transform!", point3d_transform_bang, 1);
	rb_define_singleton_method(cPoint3d, "linear_combination", point3d_linear_combination, 4);
	rb_define_method(cPoint3d, "initialize", point3d_initialize, -1);
	rb_define_method(cPoint3d, "initialize_copy", point3d_initialize_copy, 1);
	rb_define_method(cPoint3d, "set!", point3d_set_bang, -1);
	rb_define_method(cPoint3d, "x", point3d_x, 0);
	rb_define_method(cPoint3d, "x=", point3d_set_x, 1);
	rb_define_method(cPoint3d, "y", point3d_y, 0);
	rb_define_method(cPoint3d, "y=", point3d_set_y, 1);
	rb_define_method(cPoint3d, "z", point3d_z, 0);
	rb_define_method(cPoint3d, "z=", point3d_set_z, 1);
	rb_define_method(cPoint3d, "[]", point3d_aref, 1);
	rb_define_method(cPoint3d, "[]=", point3d_aset, 2);
	rb_define_method(cPoint3d, "to_a", point3d_to_a, 0);
	rb_define_method(cPoint3d, "to_s", point3d_to_s, 0);
	rb_define_method(cPoint3d, "inspect", point3d_inspect, 0);
	rb_define_method(cPoint3d, "+", point3d_plus, 1);
	rb_define_method(cPoint3d, "-", point3d_minus, 1);
	rb_define_method(cPoint3d, "<", point3d_lt, 1);
	rb_define_method(cPoint3d, "==", point3d_eq, 1);
	rb_define_method(cPoint3d, "distance", point3d_distance, 1);
	rb_define_method(cPoint3d, "vector_to", point3d_vector_to, 1);
	rb_define_method(cPoint3d, "offset", point3d_offset, -1);
	rb_define_method(cPoint3d, "offset!", point3d_offset_bang, -1);
	rb_define_method(cPoint3d, "distance_to_line", point3d_distance_to_line, 1);
	rb_define_method(cPoint3d, "on_line?", point3d_on_line_p, 1);
	rb_define_method(cPoint3d, "project_to_line", point3d_project_to_line, 1);
	rb_define_method(cPoint3d, "distance_to_plane", point3d_distance_to_plane, 1);
	rb_define_method(cPoint3d, "on_plane?", point3d_on_plane_p, 1);
	rb_define_method(cPoint3d, "project_to_plane", point3d_project_to_plane, 1);
}
