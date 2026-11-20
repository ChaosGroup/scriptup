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

static VALUE drawingelement_model(VALUE self)
{
	SUModelRef model = SU_INVALID;
	SUEntityGetModel(SU_REF(SUEntityRef, self), &model);
	return su_wrap_or_nil(cModel, model.ptr);
}

// Material, material name or Color (a new material is added to the model).
static VALUE drawingelement_set_material(VALUE self, VALUE material)
{
	SUMaterialRef ref = SU_INVALID;
	if (RB_TYPE_P(material, T_STRING))
	{
		VALUE found = rb_funcall(rb_funcall(drawingelement_model(self), (rb_intern)("materials"), 0), (rb_intern)("[]"), 1, material);
		if (NIL_P(found))
			rb_raise(rb_eArgError, "material not found: %"PRIsVALUE, material);
		ref.ptr = DATA_PTR(found);
	}
	else if (rb_obj_is_kind_of(material, cColor))
	{
		SUMaterialCreate(&ref);
		SUMaterialSetColor(ref, DATA_PTR(material));
		SUModelRef model = SU_REF(SUModelRef, drawingelement_model(self));
		SUModelAddMaterials(model, 1, &ref);
	}
	else if (!NIL_P(material))
		ref.ptr = su_unwrap(material, cMaterial);
	SUDrawingElementSetMaterial(SU_REF(SUDrawingElementRef, self), ref);
	return material;
}

// Layer or layer name.
static VALUE drawingelement_set_layer(VALUE self, VALUE layer)
{
	SULayerRef ref = SU_INVALID;
	if (RB_TYPE_P(layer, T_STRING))
	{
		SUModelRef model = SU_REF(SUModelRef, drawingelement_model(self));
		size_t count = 0;
		SUModelGetNumLayers(model, &count);
		SULayerRef* layers = ALLOC_N(SULayerRef, count > 0 ? count : 1);
		SUModelGetLayers(model, count, layers, &count);
		for (size_t i = 0; i < count && SUIsInvalid(ref); i++)
		{
			SUStringRef name = SU_INVALID;
			SUStringCreate(&name);
			SULayerGetName(layers[i], &name);
			if (rb_str_equal(su_string_to_rb(name), layer) == Qtrue)
				ref = layers[i];
			SUStringRelease(&name);
		}
		xfree(layers);
		if (SUIsInvalid(ref))
			rb_raise(rb_eArgError, "layer not found: %"PRIsVALUE, layer);
	}
	else if (!NIL_P(layer))
	{
		if (!rb_obj_is_kind_of(layer, cLayer))
			rb_raise(rb_eArgError, "Wrong type of object given");
		ref.ptr = DATA_PTR(layer);
	}
	SUDrawingElementSetLayer(SU_REF(SUDrawingElementRef, self), ref);
	return layer;
}

static VALUE drawingelement_erase_bang(VALUE self)
{
	SUEntityRef entity = SU_REF(SUEntityRef, self);
	SUEntitiesRef parent = SU_INVALID;
	if (SUEntityGetParentEntities(entity, &parent) != SU_ERROR_NONE || SUEntitiesErase(parent, 1, &entity) != SU_ERROR_NONE)
		rb_raise(rb_eTypeError, "cannot erase this entity");
	return Qnil;
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
	rb_define_method(cDrawingelement, "erase!", drawingelement_erase_bang, 0);
}
