#include <scriptup.h>

DEFINE_REF_LIST(entities_faces, SUEntitiesRef, SUEntitiesGetNumFaces, SUEntitiesGetFaces, SUFaceRef, cFace)

static VALUE entities_each(VALUE self)
{
	return su_yield_each(self, entities_faces(SU_REF(SUEntitiesRef, self)));
}

void Init_entities(void)
{
	rb_include_module(cEntities, rb_mEnumerable);
	rb_define_method(cEntities, "each", entities_each, 0);
}
