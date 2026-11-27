#include <scriptup.h>

#define VERTEX(self) SU_REF(SUVertexRef, self)

DEFINE_REF_LIST(vertex_edge_list, SUVertexRef, SUVertexGetNumEdges, SUVertexGetEdges, SUEdgeRef, cEdge)
DEFINE_REF_LIST(vertex_face_list, SUVertexRef, SUVertexGetNumFaces, SUVertexGetFaces, SUFaceRef, cFace)
DEFINE_REF_LIST(vertex_loop_list, SUVertexRef, SUVertexGetNumLoops, SUVertexGetLoops, SULoopRef, cLoop)

static VALUE vertex_edges(VALUE self) { return vertex_edge_list(VERTEX(self)); }
static VALUE vertex_faces(VALUE self) { return vertex_face_list(VERTEX(self)); }
static VALUE vertex_loops(VALUE self) { return vertex_loop_list(VERTEX(self)); }

static VALUE vertex_position(VALUE self)
{
	struct SUPoint3D p = {0, 0, 0};
	SUVertexGetPosition(VERTEX(self), &p);
	return su_point3d_new(p);
}

static VALUE vertex_common_edge(VALUE self, VALUE other)
{
	su_unwrap(other, cVertex);
	VALUE edges = vertex_edges(self);
	VALUE other_edges = vertex_edges(other);
	for (long i = 0; i < RARRAY_LEN(edges); i++)
		if (RTEST(rb_ary_includes(other_edges, RARRAY_AREF(edges, i))))
			return RARRAY_AREF(edges, i);
	return Qnil;
}

static VALUE vertex_used_by_p(VALUE self, VALUE element)
{
	if (rb_obj_is_kind_of(element, cEdge))
		return rb_ary_includes(vertex_edges(self), element);
	if (rb_obj_is_kind_of(element, cFace))
		return rb_ary_includes(vertex_faces(self), element);
	rb_raise(rb_eTypeError, "expected an Edge or a Face");
}

// The arc curve this vertex is inside of (not an end point of), if any.
static VALUE vertex_curve_interior_p(VALUE self)
{
	VALUE edges = vertex_edges(self);
	if (RARRAY_LEN(edges) != 2)
		return Qnil;
	SUCurveRef curves[2];
	for (long i = 0; i < 2; i++)
	{
		curves[i].ptr = NULL;
		SUEdgeGetCurve(SU_REF(SUEdgeRef, RARRAY_AREF(edges, i)), &curves[i]);
	}
	if (SUIsInvalid(curves[0]) || curves[0].ptr != curves[1].ptr)
		return Qnil;
	VALUE curve = su_wrap_entity((SUEntityRef){curves[0].ptr});
	return rb_obj_is_kind_of(curve, cArcCurve) ? curve : Qnil;
}

void Init_vertex(void)
{
	rb_define_method(cVertex, "position", vertex_position, 0);
	rb_define_method(cVertex, "edges", vertex_edges, 0);
	rb_define_method(cVertex, "faces", vertex_faces, 0);
	rb_define_method(cVertex, "loops", vertex_loops, 0);
	rb_define_method(cVertex, "common_edge", vertex_common_edge, 1);
	rb_define_method(cVertex, "used_by?", vertex_used_by_p, 1);
	rb_define_method(cVertex, "curve_interior?", vertex_curve_interior_p, 0);
}
