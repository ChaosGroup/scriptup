#include <stdbool.h>
#include <model.h>
#include <utils.h>
#include <SketchUpAPI/sketchup.h>
#include <sketchup.h>

static VALUE Sketchup_Model_close(VALUE self)
{
	SUModelRef model = Sketchup_get_active_model();
	enum SUResult result = SUModelRelease(&model);
	Sketchup_set_active_model(model);
	if(result != SU_ERROR_NONE)
		return false;
	return Qnil;
}

static VALUE Sketchup_Model_materials(VALUE self)
{
	return Data_Wrap_Struct(rb_path2class(SKETCHUP_MATERIALS), 0, 0, DATA_PTR(self));
}

static VALUE Sketchup_Model_definitions(VALUE self)
{
	return Data_Wrap_Struct(rb_path2class(SKETCHUP_DEFINITIONLIST), 0, 0, DATA_PTR(self));
}

static VALUE Sketchup_Model_attribute_dictionaries(VALUE self)
{
	return Data_Wrap_Struct(rb_path2class(SKETCHUP_ATTRIBUTEDICTIONARIES), 0, 0, DATA_PTR(self));
}

VALUE Model_Init(VALUE Sketchup, VALUE rb_cObject)
{
	VALUE Sketchup_Model = rb_define_class_under(Sketchup, MODEL, rb_cObject);
	rb_undef_alloc_func(Sketchup_Model);
	rb_define_const(Sketchup_Model, "LOAD_STATUS_SUCCESS", INT2FIX(0));
	rb_define_const(Sketchup_Model, "LOAD_STATUS_SUCCESS_MORE_RECENT", INT2FIX(1));
	rb_define_method(Sketchup_Model, "close", Sketchup_Model_close, 0);
	rb_define_method(Sketchup_Model, "materials", Sketchup_Model_materials, 0);
	rb_define_method(Sketchup_Model, "definitions", Sketchup_Model_definitions, 0);
	rb_define_method(Sketchup_Model, "attribute_dictionaries", Sketchup_Model_attribute_dictionaries, 0);
	return Sketchup_Model;
}