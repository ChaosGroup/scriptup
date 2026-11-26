#include <scriptup.h>

static VALUE model_close(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 0, 1);
	SUModelRef model = Sketchup_get_active_model();
	enum SUResult result = SUModelRelease(&model);
	Sketchup_set_active_model(model);
	return result == SU_ERROR_NONE ? Qnil : Qfalse;
}

static VALUE model_entities(VALUE self)
{
	SUEntitiesRef entities = SU_INVALID;
	SUModelGetEntities(SU_REF(SUModelRef, self), &entities);
	return su_entities_new(entities);
}

static VALUE model_layers(VALUE self)
{
	return su_wrap(cLayers, DATA_PTR(self));
}

static VALUE model_line_styles(VALUE self)
{
	SULineStylesRef styles = SU_INVALID;
	SUModelGetLineStyles(SU_REF(SUModelRef, self), &styles);
	return su_wrap_or_nil(cLineStyles, styles.ptr);
}

static VALUE model_materials(VALUE self)
{
	return su_wrap(cMaterials, DATA_PTR(self));
}

static VALUE model_definitions(VALUE self)
{
	return su_wrap(cDefinitionList, DATA_PTR(self));
}

static VALUE model_attribute_dictionaries(VALUE self)
{
	return su_model_attribute_dictionaries(SU_REF(SUModelRef, self));
}

static VALUE model_eq(VALUE self, VALUE other)
{
	return BOOL2RB(rb_obj_is_kind_of(other, cModel) && DATA_PTR(self) == DATA_PTR(other));
}

static VALUE model_hash(VALUE self)
{
	return rb_funcall(ULL2NUM((unsigned long long)(uintptr_t)DATA_PTR(self)), (rb_intern)("hash"), 0);
}

void Init_model(void)
{
	rb_define_method(cModel, "==", model_eq, 1);
	rb_define_method(cModel, "eql?", model_eq, 1);
	rb_define_method(cModel, "hash", model_hash, 0);
	rb_define_const(cModel, "LOAD_STATUS_SUCCESS", INT2FIX(SUModelLoadStatus_Success));
	rb_define_const(cModel, "LOAD_STATUS_SUCCESS_MORE_RECENT", INT2FIX(SUModelLoadStatus_Success_MoreRecent));
	rb_define_method(cModel, "close", model_close, -1);
	rb_define_method(cModel, "entities", model_entities, 0);
	rb_define_method(cModel, "layers", model_layers, 0);
	rb_define_method(cModel, "tags", model_layers, 0);
	rb_define_method(cModel, "line_styles", model_line_styles, 0);
	rb_define_method(cModel, "materials", model_materials, 0);
	rb_define_method(cModel, "definitions", model_definitions, 0);
	rb_define_method(cModel, "attribute_dictionaries", model_attribute_dictionaries, 0);
}
