#include <scriptup.h>

static bool not_rtest(VALUE value)
{
	return !RTEST(value);
}

DEFINE_BOOL_GETTER(drawingelement_casts_shadows_p, SUDrawingElementRef, SUDrawingElementGetCastsShadows)
DEFINE_BOOL_SETTER(drawingelement_set_casts_shadows, SUDrawingElementRef, SUDrawingElementSetCastsShadows)
DEFINE_BOOL_GETTER(drawingelement_receives_shadows_p, SUDrawingElementRef, SUDrawingElementGetReceivesShadows)
DEFINE_BOOL_SETTER(drawingelement_set_receives_shadows, SUDrawingElementRef, SUDrawingElementSetReceivesShadows)
DEFINE_BOOL_GETTER(drawingelement_hidden_p, SUDrawingElementRef, SUDrawingElementGetHidden)
DEFINE_BOOL_SETTER(drawingelement_set_hidden, SUDrawingElementRef, SUDrawingElementSetHidden)
DEFINE_SETTER(drawingelement_set_visible, SUDrawingElementRef, SUDrawingElementSetHidden, not_rtest)
DEFINE_REF_GETTER(drawingelement_material, SUDrawingElementRef, SUDrawingElementGetMaterial, SUMaterialRef, cMaterial)
DEFINE_REF_GETTER(drawingelement_layer, SUDrawingElementRef, SUDrawingElementGetLayer, SULayerRef, cLayer)

static VALUE drawingelement_visible_p(VALUE self)
{
	return RTEST(drawingelement_hidden_p(self)) ? Qfalse : Qtrue;
}

static VALUE drawingelement_bounds(VALUE self)
{
	struct SUBoundingBox3D bbox;
	SUDrawingElementGetBoundingBox(SU_REF(SUDrawingElementRef, self), &bbox);
	return su_bounding_box_new(bbox);
}

static VALUE drawingelement_set_material(VALUE self, VALUE material)
{
	SUMaterialRef ref = SU_INVALID;
	if (!NIL_P(material))
		ref.ptr = su_unwrap(material, cMaterial);
	SUDrawingElementSetMaterial(SU_REF(SUDrawingElementRef, self), ref);
	return material;
}

static VALUE drawingelement_set_layer(VALUE self, VALUE layer)
{
	if (!NIL_P(layer) && !rb_obj_is_kind_of(layer, cLayer))
		rb_raise(rb_eArgError, "Wrong type of object given");
	SULayerRef ref = SU_INVALID;
	if (!NIL_P(layer))
		ref.ptr = DATA_PTR(layer);
	SUDrawingElementSetLayer(SU_REF(SUDrawingElementRef, self), ref);
	return layer;
}

void Init_drawing_element(void)
{
	rb_define_method(cDrawingelement, "bounds", drawingelement_bounds, 0);
	rb_define_method(cDrawingelement, "casts_shadows?", drawingelement_casts_shadows_p, 0);
	rb_define_method(cDrawingelement, "casts_shadows=", drawingelement_set_casts_shadows, 1);
	rb_define_method(cDrawingelement, "receives_shadows?", drawingelement_receives_shadows_p, 0);
	rb_define_method(cDrawingelement, "receives_shadows=", drawingelement_set_receives_shadows, 1);
	rb_define_method(cDrawingelement, "hidden?", drawingelement_hidden_p, 0);
	rb_define_method(cDrawingelement, "hidden=", drawingelement_set_hidden, 1);
	rb_define_method(cDrawingelement, "visible?", drawingelement_visible_p, 0);
	rb_define_method(cDrawingelement, "visible=", drawingelement_set_visible, 1);
	rb_define_method(cDrawingelement, "material", drawingelement_material, 0);
	rb_define_method(cDrawingelement, "material=", drawingelement_set_material, 1);
	rb_define_method(cDrawingelement, "layer", drawingelement_layer, 0);
	rb_define_method(cDrawingelement, "layer=", drawingelement_set_layer, 1);
}
