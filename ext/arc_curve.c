#include <scriptup.h>

#define ARC(self) SU_REF(SUArcCurveRef, self)

DEFINE_DOUBLE_GETTER(arccurve_radius, SUArcCurveRef, SUArcCurveGetRadius)
DEFINE_DOUBLE_GETTER(arccurve_start_angle, SUArcCurveRef, SUArcCurveGetStartAngle)
DEFINE_DOUBLE_GETTER(arccurve_end_angle, SUArcCurveRef, SUArcCurveGetEndAngle)
DEFINE_BOOL_GETTER(arccurve_circular_p, SUArcCurveRef, SUArcCurveGetIsFullCircle)

static VALUE arccurve_center(VALUE self)
{
	struct SUPoint3D center = {0, 0, 0};
	SUArcCurveGetCenter(ARC(self), &center);
	return su_point3d_new(center);
}

#define DEFINE_VECTOR(fn, SUFn) \
	static VALUE fn(VALUE self) \
	{ \
		struct SUVector3D v = {0, 0, 0}; \
		SUFn(ARC(self), &v); \
		return su_vector3d_new(v); \
	}
DEFINE_VECTOR(arccurve_normal, SUArcCurveGetNormal)
DEFINE_VECTOR(arccurve_xaxis, SUArcCurveGetXAxis)
DEFINE_VECTOR(arccurve_yaxis, SUArcCurveGetYAxis)

static VALUE arccurve_plane(VALUE self)
{
	struct SUPoint3D center = {0, 0, 0};
	struct SUVector3D normal = {0, 0, 0};
	SUArcCurveGetCenter(ARC(self), &center);
	SUArcCurveGetNormal(ARC(self), &normal);
	struct SUPlane3D plane;
	SUPlane3DSetWithPointAndNormal(&plane, &center, &normal);
	return rb_ary_new_from_args(4, DBL2NUM(plane.a), DBL2NUM(plane.b), DBL2NUM(plane.c), DBL2NUM(plane.d));
}

void Init_arc_curve(void)
{
	rb_define_method(cArcCurve, "center", arccurve_center, 0);
	rb_define_method(cArcCurve, "radius", arccurve_radius, 0);
	rb_define_method(cArcCurve, "start_angle", arccurve_start_angle, 0);
	rb_define_method(cArcCurve, "end_angle", arccurve_end_angle, 0);
	rb_define_method(cArcCurve, "circular?", arccurve_circular_p, 0);
	rb_define_method(cArcCurve, "normal", arccurve_normal, 0);
	rb_define_method(cArcCurve, "xaxis", arccurve_xaxis, 0);
	rb_define_method(cArcCurve, "yaxis", arccurve_yaxis, 0);
	rb_define_method(cArcCurve, "plane", arccurve_plane, 0);
}
