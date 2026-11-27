#include <scriptup.h>
#include <float.h>

#define BB(obj) ((struct SUBoundingBox3D*)DATA_PTR(obj))

// An empty box has min > max.
static const struct SUBoundingBox3D empty_box = {{DBL_MAX, DBL_MAX, DBL_MAX}, {-DBL_MAX, -DBL_MAX, -DBL_MAX}};

static bool box_empty(const struct SUBoundingBox3D* b)
{
	return b->min_point.x > b->max_point.x || b->min_point.y > b->max_point.y || b->min_point.z > b->max_point.z;
}

static VALUE boundingbox_alloc(VALUE klass)
{
	struct SUBoundingBox3D* b;
	VALUE obj = Data_Make_Struct(klass, struct SUBoundingBox3D, 0, RUBY_DEFAULT_FREE, b);
	*b = empty_box;
	return obj;
}

VALUE su_bounding_box_new(struct SUBoundingBox3D value)
{
	VALUE obj = boundingbox_alloc(cBoundingBox);
	*BB(obj) = value;
	return obj;
}

static void box_add_point(struct SUBoundingBox3D* b, struct SUPoint3D p)
{
	b->min_point.x = fmin(b->min_point.x, p.x);
	b->min_point.y = fmin(b->min_point.y, p.y);
	b->min_point.z = fmin(b->min_point.z, p.z);
	b->max_point.x = fmax(b->max_point.x, p.x);
	b->max_point.y = fmax(b->max_point.y, p.y);
	b->max_point.z = fmax(b->max_point.z, p.z);
}

static void box_add(struct SUBoundingBox3D* b, VALUE item)
{
	if (rb_obj_is_kind_of(item, cBoundingBox))
	{
		if (!box_empty(BB(item)))
		{
			box_add_point(b, BB(item)->min_point);
			box_add_point(b, BB(item)->max_point);
		}
	}
	else if (RB_TYPE_P(item, T_ARRAY) && RARRAY_LEN(item) > 0 && !rb_obj_is_kind_of(RARRAY_AREF(item, 0), rb_cNumeric))
	{
		for (long i = 0; i < RARRAY_LEN(item); i++)
			box_add(b, RARRAY_AREF(item, i));
	}
	else
		box_add_point(b, su_to_point3d(item));
}

static VALUE boundingbox_initialize(VALUE self)
{
	return self;
}

static VALUE boundingbox_initialize_copy(VALUE self, VALUE other)
{
	*BB(self) = *BB(other);
	return self;
}

static VALUE boundingbox_add(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 1, UNLIMITED_ARGUMENTS);
	for (int i = 0; i < argc; i++)
		box_add(BB(self), argv[i]);
	return self;
}

static VALUE boundingbox_clear(VALUE self)
{
	*BB(self) = empty_box;
	return self;
}

static VALUE boundingbox_empty_p(VALUE self)
{
	return BOOL2RB(box_empty(BB(self)));
}

static VALUE boundingbox_valid_p(VALUE self)
{
	return BOOL2RB(!box_empty(BB(self)));
}

static VALUE boundingbox_min(VALUE self)
{
	return su_point3d_new(BB(self)->min_point);
}

static VALUE boundingbox_max(VALUE self)
{
	return su_point3d_new(BB(self)->max_point);
}

static VALUE boundingbox_center(VALUE self)
{
	struct SUPoint3D mid = {0, 0, 0};
	if (!box_empty(BB(self)))
		SUBoundingBox3DGetMidPoint(BB(self), &mid);
	return su_point3d_new(mid);
}

static double extent(VALUE self, int axis)
{
	struct SUBoundingBox3D* b = BB(self);
	if (box_empty(b))
		return 0.0;
	double lo[3] = {b->min_point.x, b->min_point.y, b->min_point.z};
	double hi[3] = {b->max_point.x, b->max_point.y, b->max_point.z};
	return hi[axis] - lo[axis];
}

static VALUE boundingbox_width(VALUE self) { return su_length_new(extent(self, 0)); }
static VALUE boundingbox_height(VALUE self) { return su_length_new(extent(self, 1)); }
static VALUE boundingbox_depth(VALUE self) { return su_length_new(extent(self, 2)); }

static VALUE boundingbox_diagonal(VALUE self)
{
	double x = extent(self, 0);
	double y = extent(self, 1);
	double z = extent(self, 2);
	return su_length_new(sqrt(x * x + y * y + z * z));
}

static VALUE boundingbox_corner(VALUE self, VALUE index)
{
	int i = NUM2INT(index);
	if (i < 0 || i > 7)
		rb_raise(rb_eIndexError, "corner index %d out of range [0, 7]", i);
	struct SUBoundingBox3D* b = BB(self);
	return su_point3d_new((struct SUPoint3D){
		(i & 1) ? b->max_point.x : b->min_point.x,
		(i & 2) ? b->max_point.y : b->min_point.y,
		(i & 4) ? b->max_point.z : b->min_point.z});
}

static bool point_inside(const struct SUBoundingBox3D* b, struct SUPoint3D p)
{
	return p.x >= b->min_point.x && p.x <= b->max_point.x && p.y >= b->min_point.y && p.y <= b->max_point.y &&
		p.z >= b->min_point.z && p.z <= b->max_point.z;
}

static VALUE boundingbox_contains_p(VALUE self, VALUE other)
{
	struct SUBoundingBox3D* b = BB(self);
	if (box_empty(b))
		return Qfalse;
	if (rb_obj_is_kind_of(other, cBoundingBox))
		return BOOL2RB(!box_empty(BB(other)) && point_inside(b, BB(other)->min_point) && point_inside(b, BB(other)->max_point));
	return BOOL2RB(point_inside(b, su_to_point3d(other)));
}

static VALUE boundingbox_intersect(VALUE self, VALUE other)
{
	struct SUBoundingBox3D* a = BB(self);
	struct SUBoundingBox3D* b = su_unwrap(other, cBoundingBox);
	struct SUBoundingBox3D result = {
		{fmax(a->min_point.x, b->min_point.x), fmax(a->min_point.y, b->min_point.y), fmax(a->min_point.z, b->min_point.z)},
		{fmin(a->max_point.x, b->max_point.x), fmin(a->max_point.y, b->max_point.y), fmin(a->max_point.z, b->max_point.z)}};
	return su_bounding_box_new(box_empty(&result) ? empty_box : result);
}

void Init_bounding_box(void)
{
	static const char* corners[8] = {"CORNER_LEFT_FRONT_BOTTOM", "CORNER_RIGHT_FRONT_BOTTOM", "CORNER_LEFT_BACK_BOTTOM",
		"CORNER_RIGHT_BACK_BOTTOM", "CORNER_LEFT_FRONT_TOP", "CORNER_RIGHT_FRONT_TOP", "CORNER_LEFT_BACK_TOP",
		"CORNER_RIGHT_BACK_TOP"};
	for (int i = 0; i < 8; i++)
		rb_define_const(cBoundingBox, corners[i], INT2FIX(i));
	rb_define_alloc_func(cBoundingBox, boundingbox_alloc);
	rb_define_method(cBoundingBox, "initialize", boundingbox_initialize, 0);
	rb_define_method(cBoundingBox, "initialize_copy", boundingbox_initialize_copy, 1);
	rb_define_method(cBoundingBox, "add", boundingbox_add, -1);
	rb_define_method(cBoundingBox, "clear", boundingbox_clear, 0);
	rb_define_method(cBoundingBox, "empty?", boundingbox_empty_p, 0);
	rb_define_method(cBoundingBox, "valid?", boundingbox_valid_p, 0);
	rb_define_method(cBoundingBox, "min", boundingbox_min, 0);
	rb_define_method(cBoundingBox, "max", boundingbox_max, 0);
	rb_define_method(cBoundingBox, "center", boundingbox_center, 0);
	rb_define_method(cBoundingBox, "width", boundingbox_width, 0);
	rb_define_method(cBoundingBox, "height", boundingbox_height, 0);
	rb_define_method(cBoundingBox, "depth", boundingbox_depth, 0);
	rb_define_method(cBoundingBox, "diagonal", boundingbox_diagonal, 0);
	rb_define_method(cBoundingBox, "corner", boundingbox_corner, 1);
	rb_define_method(cBoundingBox, "contains?", boundingbox_contains_p, 1);
	rb_define_method(cBoundingBox, "intersect", boundingbox_intersect, 1);
}
