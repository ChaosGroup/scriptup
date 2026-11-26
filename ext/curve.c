#include <scriptup.h>

#define CURVE(self) SU_REF(SUCurveRef, self)

DEFINE_REF_LIST(curve_edge_list, SUCurveRef, SUCurveGetNumEdges, SUCurveGetEdges, SUEdgeRef, cEdge)
DEFINE_SIZE_GETTER(curve_count_edges, SUCurveRef, SUCurveGetNumEdges)
DEFINE_BOOL_GETTER(curve_is_polygon_p, SUCurveRef, SUCurveIsPolygon)

static VALUE curve_edges(VALUE self)
{
	return curve_edge_list(CURVE(self));
}

static VALUE curve_each_edge(VALUE self)
{
	su_yield_each(self, curve_edges(self));
	return Qnil;
}

static VALUE curve_first_edge(VALUE self)
{
	return rb_ary_entry(curve_edges(self), 0);
}

static VALUE curve_last_edge(VALUE self)
{
	return rb_ary_entry(curve_edges(self), -1);
}

static VALUE curve_length(VALUE self)
{
	VALUE edges = curve_edges(self);
	double total = 0.0;
	for (long i = 0; i < RARRAY_LEN(edges); i++)
		total += NUM2DBL(rb_funcall(RARRAY_AREF(edges, i), (rb_intern)("length"), 0));
	return su_length_new(total);
}

// Ordered vertices: the start of each edge, plus the end of the last unless the curve is closed.
static VALUE curve_vertices(VALUE self)
{
	VALUE edges = curve_edges(self);
	VALUE vertices = rb_ary_new();
	for (long i = 0; i < RARRAY_LEN(edges); i++)
	{
		SUVertexRef start = SU_INVALID;
		SUEdgeGetStartVertex(SU_REF(SUEdgeRef, RARRAY_AREF(edges, i)), &start);
		rb_ary_push(vertices, su_wrap(cVertex, start.ptr));
	}
	if (RARRAY_LEN(edges) > 0)
	{
		SUVertexRef end = SU_INVALID;
		SUEdgeGetEndVertex(SU_REF(SUEdgeRef, RARRAY_AREF(edges, RARRAY_LEN(edges) - 1)), &end);
		VALUE last = su_wrap(cVertex, end.ptr);
		if (!RTEST(rb_ary_includes(vertices, last)))
			rb_ary_push(vertices, last);
	}
	return vertices;
}

static VALUE curve_move_vertices(VALUE self, VALUE points)
{
	Check_Type(points, T_ARRAY);
	VALUE vertices = curve_vertices(self);
	if (RARRAY_LEN(points) != RARRAY_LEN(vertices))
		rb_raise(rb_eArgError, "expected %ld points, got %ld", RARRAY_LEN(vertices), RARRAY_LEN(points));
	for (long i = 0; i < RARRAY_LEN(points); i++)
	{
		struct SUPoint3D p = su_to_point3d(RARRAY_AREF(points, i));
		if (SUVertexSetPosition(SU_REF(SUVertexRef, RARRAY_AREF(vertices, i)), &p) != SU_ERROR_NONE)
			return Qfalse;
	}
	return Qtrue;
}

void Init_curve(void)
{
	rb_define_method(cCurve, "edges", curve_edges, 0);
	rb_define_method(cCurve, "each_edge", curve_each_edge, 0);
	rb_define_method(cCurve, "count_edges", curve_count_edges, 0);
	rb_define_method(cCurve, "first_edge", curve_first_edge, 0);
	rb_define_method(cCurve, "last_edge", curve_last_edge, 0);
	rb_define_method(cCurve, "is_polygon?", curve_is_polygon_p, 0);
	rb_define_method(cCurve, "length", curve_length, 0);
	rb_define_method(cCurve, "vertices", curve_vertices, 0);
	rb_define_method(cCurve, "move_vertices", curve_move_vertices, 1);
}
