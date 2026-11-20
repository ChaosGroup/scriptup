#include <scriptup.h>

#define GEOM_TOLERANCE 1.0e-3

static struct SUVector3D sub(struct SUPoint3D a, struct SUPoint3D b)
{
	return (struct SUVector3D){a.x - b.x, a.y - b.y, a.z - b.z};
}

static double dot(struct SUVector3D a, struct SUVector3D b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static struct SUPoint3D along(struct SURay3D ray, double t)
{
	return (struct SUPoint3D){ray.point.x + ray.normal.x * t, ray.point.y + ray.normal.y * t, ray.point.z + ray.normal.z * t};
}

// Closest points between two lines; false if they are parallel.
static bool closest(struct SURay3D a, struct SURay3D b, struct SUPoint3D* pa, struct SUPoint3D* pb)
{
	struct SUVector3D w = sub(a.point, b.point);
	double aa = dot(a.normal, a.normal);
	double ab = dot(a.normal, b.normal);
	double bb = dot(b.normal, b.normal);
	double aw = dot(a.normal, w);
	double bw = dot(b.normal, w);
	double denominator = aa * bb - ab * ab;
	if (fabs(denominator) < 1.0e-12)
		return false;
	*pa = along(a, (ab * bw - bb * aw) / denominator);
	*pb = along(b, (aa * bw - ab * aw) / denominator);
	return true;
}

static VALUE geom_closest_points(VALUE self, VALUE line1, VALUE line2)
{
	struct SURay3D a = su_to_ray3d(line1);
	struct SURay3D b = su_to_ray3d(line2);
	struct SUPoint3D pa;
	struct SUPoint3D pb;
	if (!closest(a, b, &pa, &pb))
	{
		pa = a.point;
		SURay3DProjectTo(&b, &pa, &pb);
	}
	return rb_ary_new_from_args(2, su_point3d_new(pa), su_point3d_new(pb));
}

static VALUE geom_intersect_line_line(VALUE self, VALUE line1, VALUE line2)
{
	struct SUPoint3D pa;
	struct SUPoint3D pb;
	if (!closest(su_to_ray3d(line1), su_to_ray3d(line2), &pa, &pb))
		return Qnil;
	double distance = 0.0;
	SUPoint3DDistanceToSUPoint3D(&pa, &pb, &distance);
	return distance < GEOM_TOLERANCE ? su_point3d_new(pa) : Qnil;
}

static VALUE geom_intersect_line_plane(VALUE self, VALUE line, VALUE plane)
{
	struct SURay3D ray = su_to_ray3d(line);
	struct SUPlane3D p = su_to_plane3d(plane);
	struct SUVector3D normal = {p.a, p.b, p.c};
	double denominator = dot(normal, ray.normal);
	if (fabs(denominator) < 1.0e-12)
		return Qnil;
	double t = -(dot(normal, (struct SUVector3D){ray.point.x, ray.point.y, ray.point.z}) + p.d) / denominator;
	return su_point3d_new(along(ray, t));
}

static VALUE geom_intersect_plane_plane(VALUE self, VALUE plane1, VALUE plane2)
{
	struct SUPlane3D p = su_to_plane3d(plane1);
	struct SUPlane3D q = su_to_plane3d(plane2);
	struct SUVector3D n1 = {p.a, p.b, p.c};
	struct SUVector3D n2 = {q.a, q.b, q.c};
	struct SUVector3D direction;
	SUVector3DCross(&n1, &n2, &direction);
	double length = sqrt(dot(direction, direction));
	if (length < 1.0e-12)
		return Qnil;
	// Point on both planes: ((d2 n1 - d1 n2) x direction) / |direction|^2
	struct SUVector3D v = {q.d * n1.x - p.d * n2.x, q.d * n1.y - p.d * n2.y, q.d * n1.z - p.d * n2.z};
	struct SUVector3D c;
	SUVector3DCross(&v, &direction, &c);
	double l2 = length * length;
	struct SUPoint3D point = {c.x / l2, c.y / l2, c.z / l2};
	SUVector3DNormalize(&direction);
	return rb_ary_new_from_args(2, su_point3d_new(point), su_vector3d_new(direction));
}

static VALUE geom_linear_combination(VALUE self, VALUE w1, VALUE a, VALUE w2, VALUE b)
{
	if (rb_obj_is_kind_of(a, cVector3d))
	{
		struct SUVector3D va = su_to_vector3d(a);
		struct SUVector3D vb = su_to_vector3d(b);
		double x = NUM2DBL(w1);
		double y = NUM2DBL(w2);
		return su_vector3d_new((struct SUVector3D){x * va.x + y * vb.x, x * va.y + y * vb.y, x * va.z + y * vb.z});
	}
	return rb_funcall(cPoint3d, (rb_intern)("linear_combination"), 4, w1, a, w2, b);
}

static VALUE geom_fit_plane_to_points(int argc, VALUE* argv, VALUE self)
{
	VALUE points = argc == 1 && RB_TYPE_P(argv[0], T_ARRAY) && !rb_obj_is_kind_of(RARRAY_AREF(argv[0], 0), rb_cNumeric)
		? argv[0] : rb_ary_new_from_values(argc, argv);
	long count = RARRAY_LEN(points);
	if (count < 3)
		rb_raise(rb_eArgError, "at least 3 points are required");
	// Newell's method: normal from the polygon, plane through the centroid.
	struct SUVector3D normal = {0, 0, 0};
	struct SUPoint3D centroid = {0, 0, 0};
	for (long i = 0; i < count; i++)
	{
		struct SUPoint3D a = su_to_point3d(RARRAY_AREF(points, i));
		struct SUPoint3D b = su_to_point3d(RARRAY_AREF(points, (i + 1) % count));
		normal.x += (a.y - b.y) * (a.z + b.z);
		normal.y += (a.z - b.z) * (a.x + b.x);
		normal.z += (a.x - b.x) * (a.y + b.y);
		centroid.x += a.x / count;
		centroid.y += a.y / count;
		centroid.z += a.z / count;
	}
	struct SUPlane3D plane;
	if (SUPlane3DSetWithPointAndNormal(&plane, &centroid, &normal) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "points are collinear");
	return rb_ary_new_from_args(4, DBL2NUM(plane.a), DBL2NUM(plane.b), DBL2NUM(plane.c), DBL2NUM(plane.d));
}

static bool on_segment(double px, double py, double ax, double ay, double bx, double by)
{
	double cross = (bx - ax) * (py - ay) - (by - ay) * (px - ax);
	double length = hypot(bx - ax, by - ay);
	if (length < 1.0e-12)
		return hypot(px - ax, py - ay) < GEOM_TOLERANCE;
	return fabs(cross) / length < GEOM_TOLERANCE && px >= fmin(ax, bx) - GEOM_TOLERANCE &&
		px <= fmax(ax, bx) + GEOM_TOLERANCE && py >= fmin(ay, by) - GEOM_TOLERANCE && py <= fmax(ay, by) + GEOM_TOLERANCE;
}

static VALUE geom_point_in_polygon_2D(VALUE self, VALUE point, VALUE polygon, VALUE check_border)
{
	struct SUPoint3D p = su_to_point3d(point);
	Check_Type(polygon, T_ARRAY);
	long count = RARRAY_LEN(polygon);
	bool inside = false;
	for (long i = 0, j = count - 1; i < count; j = i++)
	{
		struct SUPoint3D a = su_to_point3d(RARRAY_AREF(polygon, i));
		struct SUPoint3D b = su_to_point3d(RARRAY_AREF(polygon, j));
		if (on_segment(p.x, p.y, a.x, a.y, b.x, b.y))
			return BOOL2RB(RTEST(check_border));
		if ((a.y > p.y) != (b.y > p.y) && p.x < (b.x - a.x) * (p.y - a.y) / (b.y - a.y) + a.x)
			inside = !inside;
	}
	return BOOL2RB(inside);
}

void Init_geom(void)
{
	rb_define_singleton_method(mGeom, "closest_points", geom_closest_points, 2);
	rb_define_singleton_method(mGeom, "fit_plane_to_points", geom_fit_plane_to_points, -1);
	rb_define_singleton_method(mGeom, "intersect_line_line", geom_intersect_line_line, 2);
	rb_define_singleton_method(mGeom, "intersect_line_plane", geom_intersect_line_plane, 2);
	rb_define_singleton_method(mGeom, "intersect_plane_plane", geom_intersect_plane_plane, 2);
	rb_define_singleton_method(mGeom, "linear_combination", geom_linear_combination, 4);
	rb_define_singleton_method(mGeom, "point_in_polygon_2D", geom_point_in_polygon_2D, 3);
}
