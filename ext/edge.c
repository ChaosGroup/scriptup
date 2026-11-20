#include <scriptup.h>

#define EDGE(self) SU_REF(SUEdgeRef, self)

DEFINE_REF_LIST(edge_face_list, SUEdgeRef, SUEdgeGetNumFaces, SUEdgeGetFaces, SUFaceRef, cFace)
DEFINE_REF_GETTER(edge_start, SUEdgeRef, SUEdgeGetStartVertex, SUVertexRef, cVertex)
DEFINE_REF_GETTER(edge_end, SUEdgeRef, SUEdgeGetEndVertex, SUVertexRef, cVertex)
DEFINE_BOOL_GETTER(edge_soft_p, SUEdgeRef, SUEdgeGetSoft)
DEFINE_BOOL_SETTER(edge_set_soft, SUEdgeRef, SUEdgeSetSoft)
DEFINE_BOOL_GETTER(edge_smooth_p, SUEdgeRef, SUEdgeGetSmooth)
DEFINE_BOOL_SETTER(edge_set_smooth, SUEdgeRef, SUEdgeSetSmooth)

static VALUE edge_faces(VALUE self)
{
	return edge_face_list(EDGE(self));
}

static VALUE edge_vertices(VALUE self)
{
	return rb_ary_new_from_args(2, edge_start(self), edge_end(self));
}

static struct SUPoint3D vertex_position(VALUE vertex)
{
	struct SUPoint3D p = {0, 0, 0};
	SUVertexGetPosition(SU_REF(SUVertexRef, vertex), &p);
	return p;
}

static VALUE edge_curve(VALUE self)
{
	SUCurveRef curve = SU_INVALID;
	SUEdgeGetCurve(EDGE(self), &curve);
	return su_wrap_entity((SUEntityRef){curve.ptr});
}

static VALUE edge_length(int argc, VALUE* argv, VALUE self)
{
	VALUE transformation;
	rb_scan_args(argc, argv, "01", &transformation);
	struct SUTransformation t = {{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}};
	if (!NIL_P(transformation))
		t = su_to_transformation(transformation);
	double length = 0.0;
	SUEdgeGetLengthWithTransform(EDGE(self), &t, &length);
	return su_length_new(length);
}

static VALUE edge_line(VALUE self)
{
	struct SUPoint3D a = vertex_position(edge_start(self));
	struct SUPoint3D b = vertex_position(edge_end(self));
	struct SUVector3D v;
	SUVector3DCreate(&v, &a, &b);
	SUVector3DNormalize(&v);
	return rb_ary_new_from_args(2, su_point3d_new(a), su_vector3d_new(v));
}

static VALUE edge_other_vertex(VALUE self, VALUE vertex)
{
	su_unwrap(vertex, cVertex);
	VALUE start = edge_start(self);
	VALUE end = edge_end(self);
	if (rb_equal(vertex, start))
		return end;
	if (rb_equal(vertex, end))
		return start;
	rb_raise(rb_eArgError, "vertex is not used by this edge");
}

static VALUE edge_reversed_in_p(VALUE self, VALUE face)
{
	bool reversed = false;
	if (SUEdgeReversedInFace(EDGE(self), (SUFaceRef){su_unwrap(face, cFace)}, &reversed) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "edge is not used by this face");
	return BOOL2RB(reversed);
}

static VALUE edge_common_face(VALUE self, VALUE other)
{
	su_unwrap(other, cEdge);
	VALUE faces = edge_faces(self);
	VALUE other_faces = edge_faces(other);
	for (long i = 0; i < RARRAY_LEN(faces); i++)
		if (RTEST(rb_ary_includes(other_faces, RARRAY_AREF(faces, i))))
			return RARRAY_AREF(faces, i);
	return Qnil;
}

static VALUE edge_used_by_p(VALUE self, VALUE element)
{
	if (rb_obj_is_kind_of(element, cVertex))
		return rb_ary_includes(edge_vertices(self), element);
	if (rb_obj_is_kind_of(element, cFace))
		return rb_ary_includes(edge_faces(self), element);
	rb_raise(rb_eTypeError, "expected a Vertex or a Face");
}

// Every edge and face reachable through shared vertices.
static VALUE edge_all_connected(VALUE self)
{
	VALUE seen = rb_hash_new();
	VALUE result = rb_ary_new();
	VALUE queue = rb_ary_new_from_args(1, self);
	while (RARRAY_LEN(queue) > 0)
	{
		VALUE edge = rb_ary_shift(queue);
		if (RTEST(rb_hash_lookup2(seen, edge, Qfalse)))
			continue;
		rb_hash_aset(seen, edge, Qtrue);
		rb_ary_push(result, edge);
		VALUE faces = edge_faces(edge);
		for (long i = 0; i < RARRAY_LEN(faces); i++)
		{
			VALUE face = RARRAY_AREF(faces, i);
			if (!RTEST(rb_hash_lookup2(seen, face, Qfalse)))
			{
				rb_hash_aset(seen, face, Qtrue);
				rb_ary_push(result, face);
			}
		}
		VALUE vertices = edge_vertices(edge);
		for (long i = 0; i < 2; i++)
			rb_ary_concat(queue, rb_funcall(RARRAY_AREF(vertices, i), (rb_intern)("edges"), 0));
	}
	return result;
}

void Init_edge(void)
{
	rb_define_method(cEdge, "start", edge_start, 0);
	rb_define_method(cEdge, "end", edge_end, 0);
	rb_define_method(cEdge, "vertices", edge_vertices, 0);
	rb_define_method(cEdge, "faces", edge_faces, 0);
	rb_define_method(cEdge, "curve", edge_curve, 0);
	rb_define_method(cEdge, "length", edge_length, -1);
	rb_define_method(cEdge, "line", edge_line, 0);
	rb_define_method(cEdge, "other_vertex", edge_other_vertex, 1);
	rb_define_method(cEdge, "reversed_in?", edge_reversed_in_p, 1);
	rb_define_method(cEdge, "common_face", edge_common_face, 1);
	rb_define_method(cEdge, "used_by?", edge_used_by_p, 1);
	rb_define_method(cEdge, "all_connected", edge_all_connected, 0);
	rb_define_method(cEdge, "soft?", edge_soft_p, 0);
	rb_define_method(cEdge, "soft=", edge_set_soft, 1);
	rb_define_method(cEdge, "smooth?", edge_smooth_p, 0);
	rb_define_method(cEdge, "smooth=", edge_set_smooth, 1);
}
