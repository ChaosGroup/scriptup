#include <scriptup.h>

#define LOOP(self) SU_REF(SULoopRef, self)

DEFINE_REF_LIST(loop_vertex_list, SULoopRef, SULoopGetNumVertices, SULoopGetVertices, SUVertexRef, cVertex)
DEFINE_REF_LIST(loop_edge_list, SULoopRef, SULoopGetNumVertices, SULoopGetEdges, SUEdgeRef, cEdge)
DEFINE_REF_LIST(loop_edgeuse_list, SULoopRef, SULoopGetNumVertices, SULoopGetEdgeUses, SUEdgeUseRef, cEdgeUse)
DEFINE_REF_GETTER(loop_face, SULoopRef, SULoopGetFace, SUFaceRef, cFace)
DEFINE_BOOL_GETTER(loop_convex_p, SULoopRef, SULoopIsConvex)
DEFINE_BOOL_GETTER(loop_outer_p, SULoopRef, SULoopIsOuterLoop)

static VALUE loop_vertices(VALUE self) { return loop_vertex_list(LOOP(self)); }
static VALUE loop_edges(VALUE self) { return loop_edge_list(LOOP(self)); }
static VALUE loop_edgeuses(VALUE self) { return loop_edgeuse_list(LOOP(self)); }

void Init_loop(void)
{
	rb_define_method(cLoop, "vertices", loop_vertices, 0);
	rb_define_method(cLoop, "edges", loop_edges, 0);
	rb_define_method(cLoop, "edgeuses", loop_edgeuses, 0);
	rb_define_method(cLoop, "face", loop_face, 0);
	rb_define_method(cLoop, "convex?", loop_convex_p, 0);
	rb_define_method(cLoop, "outer?", loop_outer_p, 0);
}
