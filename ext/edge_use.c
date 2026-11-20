#include <scriptup.h>

DEFINE_REF_GETTER(edgeuse_edge, SUEdgeUseRef, SUEdgeUseGetEdge, SUEdgeRef, cEdge)
DEFINE_REF_GETTER(edgeuse_face, SUEdgeUseRef, SUEdgeUseGetFace, SUFaceRef, cFace)
DEFINE_REF_GETTER(edgeuse_loop, SUEdgeUseRef, SUEdgeUseGetLoop, SULoopRef, cLoop)
DEFINE_REF_GETTER(edgeuse_next, SUEdgeUseRef, SUEdgeUseGetNext, SUEdgeUseRef, cEdgeUse)
DEFINE_REF_GETTER(edgeuse_previous, SUEdgeUseRef, SUEdgeUseGetPrevious, SUEdgeUseRef, cEdgeUse)
DEFINE_REF_LIST(edgeuse_partner_list, SUEdgeUseRef, SUEdgeUseGetNumPartners, SUEdgeUseGetPartners, SUEdgeUseRef, cEdgeUse)
DEFINE_BOOL_GETTER(edgeuse_reversed_p, SUEdgeUseRef, SUEdgeUseIsReversed)

static VALUE edgeuse_partners(VALUE self)
{
	return edgeuse_partner_list(SU_REF(SUEdgeUseRef, self));
}

#define DEFINE_NORMAL(fn, SUFn) \
	static VALUE fn(VALUE self) \
	{ \
		struct SUVector3D normal = {0, 0, 0}; \
		SUFn(SU_REF(SUEdgeUseRef, self), &normal); \
		return su_vector3d_new(normal); \
	}
DEFINE_NORMAL(edgeuse_start_vertex_normal, SUEdgeUseGetStartVertexNormal)
DEFINE_NORMAL(edgeuse_end_vertex_normal, SUEdgeUseGetEndVertexNormal)

void Init_edge_use(void)
{
	rb_define_method(cEdgeUse, "edge", edgeuse_edge, 0);
	rb_define_method(cEdgeUse, "face", edgeuse_face, 0);
	rb_define_method(cEdgeUse, "loop", edgeuse_loop, 0);
	rb_define_method(cEdgeUse, "next", edgeuse_next, 0);
	rb_define_method(cEdgeUse, "previous", edgeuse_previous, 0);
	rb_define_method(cEdgeUse, "partners", edgeuse_partners, 0);
	rb_define_method(cEdgeUse, "reversed?", edgeuse_reversed_p, 0);
	rb_define_method(cEdgeUse, "start_vertex_normal", edgeuse_start_vertex_normal, 0);
	rb_define_method(cEdgeUse, "end_vertex_normal", edgeuse_end_vertex_normal, 0);
}
