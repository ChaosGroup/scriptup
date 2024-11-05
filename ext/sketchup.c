#include <ruby.h>
#include <stdbool.h>
#include <sketchup.h>
#include <entity.h>
#include <entities.h>
#include <model.h>
#include <material.h>
#include <materials.h>
#include <texture.h>
#include <color.h>
#include <attribute_dictionary.h>
#include <attribute_dictionaries.h>
#include <component_definition.h>
#include <component_instance.h>
#include <definition_list.h>
#include <drawing_element.h>
#include <behavior.h>
#include <utils.h>

SUModelRef active_model = SU_INVALID;

SUModelRef Sketchup_get_active_model()
{
	return active_model;
}
void Sketchup_set_active_model(SUModelRef model)
{
	active_model = model;
}

static VALUE Sketchup_open_file(int argc, VALUE *argv, VALUE self)
{
	VALUE path;
	VALUE kwarg_hash;
	rb_scan_args(argc, argv, "1:", &path, &kwarg_hash);

	const VALUE kwarg_ids[1] = {
		rb_intern_const("with_status")
	};
	VALUE kwargs[1];
	rb_get_kwargs(kwarg_hash, kwarg_ids, 0, 1, kwargs);
	bool with_status = true;
	if (RDEF(kwargs[0]))
		with_status = RTEST(kwargs[0]);

	enum SUModelLoadStatus status;
	SUModelRef model = SU_INVALID;
	enum SUResult result = SUModelCreateFromFileWithStatus(&model, StringValuePtr(path), &status);
	if (result != SU_ERROR_NONE)
		return false;

	Sketchup_set_active_model(model);
	if (with_status)
		return INT2FIX(status);
	else
		return Qtrue;
}

static VALUE Sketchup_active_model(VALUE self)
{
	if (SUIsValid(active_model))
		return Data_Wrap_Struct(rb_path2class(SKETCHUP_MODEL), 0, 0, active_model.ptr);
	else
		return Qnil;
}

void Init_sketchup() {
	SUInitialize();
	VALUE Sketchup = rb_define_module(SKETCHUP);
	rb_define_singleton_method(Sketchup, "open_file", Sketchup_open_file, -1);
	rb_define_singleton_method(Sketchup, "active_model", Sketchup_active_model, 0);

	Model_Init(Sketchup, rb_cObject);
	Color_Init(Sketchup, rb_cObject);

	Entities_Init(Sketchup, rb_cObject);
	VALUE Sketchup_Entity = Entity_Init(Sketchup, rb_cObject);
	Materials_Init(Sketchup, Sketchup_Entity);
	Material_Init(Sketchup, Sketchup_Entity);
	DefinitionList_Init(Sketchup, Sketchup_Entity);
	AttributeDictionary_Init(Sketchup, Sketchup_Entity);
	AttributeDictionaries_Init(Sketchup, Sketchup_Entity);
	Texture_Init(Sketchup, Sketchup_Entity);
	Behavior_Init(Sketchup, Sketchup_Entity);
	
	VALUE Sketchup_DrawingElement = DrawingElement_Init(Sketchup, Sketchup_Entity);
	ComponentInstance_Init(Sketchup, Sketchup_DrawingElement);
	ComponentDefinition_Init(Sketchup, Sketchup_DrawingElement);
}