#include <scriptup.h>

static void uvhelper_free(void* ptr)
{
	SUUVHelperRef helper = {ptr};
	if (ptr)
		SUUVHelperRelease(&helper);
}

// Face#get_UVHelper(front = true, back = true, texture_writer = nil)
static VALUE face_get_UVHelper(int argc, VALUE* argv, VALUE self)
{
	VALUE front, back, writer;
	rb_scan_args(argc, argv, "03", &front, &back, &writer);
	SUTextureWriterRef texture_writer = {NIL_P(writer) ? NULL : su_unwrap(writer, cTextureWriter)};
	SUUVHelperRef helper = SU_INVALID;
	if (SUFaceGetUVHelper(SU_REF(SUFaceRef, self), NIL_P(front) || RTEST(front), NIL_P(back) || RTEST(back),
		texture_writer, &helper) != SU_ERROR_NONE)
		rb_raise(rb_eRuntimeError, "cannot create a UV helper for this face");
	return Data_Wrap_Struct(cUVHelper, 0, uvhelper_free, helper.ptr);
}

#define DEFINE_UVQ(fn, SUFn) \
	static VALUE fn(VALUE self, VALUE point) \
	{ \
		struct SUPoint3D p = su_to_point3d(point); \
		struct SUUVQ uvq = {0, 0, 1}; \
		if (SUFn(SU_REF(SUUVHelperRef, self), &p, &uvq) != SU_ERROR_NONE) \
			rb_raise(rb_eArgError, "cannot compute UVQ for this point"); \
		return su_point3d_new((struct SUPoint3D){uvq.u, uvq.v, uvq.q}); \
	}
DEFINE_UVQ(uvhelper_get_front_UVQ, SUUVHelperGetFrontUVQ)
DEFINE_UVQ(uvhelper_get_back_UVQ, SUUVHelperGetBackUVQ)

void Init_uv_helper(void)
{
	rb_undef_method(rb_singleton_class(cUVHelper), "new");
	rb_define_method(cUVHelper, "get_front_UVQ", uvhelper_get_front_UVQ, 1);
	rb_define_method(cUVHelper, "get_back_UVQ", uvhelper_get_back_UVQ, 1);
	rb_define_method(cFace, "get_UVHelper", face_get_UVHelper, -1);
}
