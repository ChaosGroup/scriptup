#include <stdbool.h>
#include <component_definition.h>
#include <utils.h>
#include <SketchUpAPI/sketchup.h>

static VALUE Sketchup_ComponentDefinition_Get_name(VALUE self)
{
	VALUE output;
	SUComponentDefinitionRef definition = {DATA_PTR(self)};
	GETSTRING(SUComponentDefinitionGetName, definition, output);
	return output;
}

static VALUE Sketchup_ComponentDefinition_Set_name(VALUE self, VALUE name)
{
	SUComponentDefinitionRef definition = {DATA_PTR(self)};
	SUComponentDefinitionSetName(definition, StringValuePtr(name));
	return name;
}

static VALUE Sketchup_ComponentDefinition_Name_equ(VALUE self, VALUE other)
{
	if (!rb_obj_is_kind_of(other, rb_path2class(SKETCHUP_COMPONENTDEFINITION)))
		return Qnil;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcompound-token-split-by-macro"
	return rb_funcall(Sketchup_ComponentDefinition_Get_name(self), rb_intern("<=>"), 1, Sketchup_ComponentDefinition_Get_name(other));
#pragma clang diagnostic pop
}

static VALUE Sketchup_ComponentDefinition_Object_equ(VALUE self, VALUE other)
{
	if (!rb_obj_is_kind_of(other, rb_path2class(SKETCHUP_COMPONENTDEFINITION)))
		return Qfalse;
	SUComponentDefinitionRef definition1 = {DATA_PTR(self)};
	SUComponentDefinitionRef definition2 = {DATA_PTR(other)};
	return definition1.ptr == definition2.ptr;
}

void Sketchup_ComponentDefinition_instances_Iterator(SUComponentInstanceRef componentInstance, VALUE ary)
{
	rb_ary_push(ary, Data_Wrap_Struct(rb_path2class(SKETCHUP_COMPONENTINSTANCE), 0, 0, componentInstance.ptr));
}

static VALUE Sketchup_ComponentDefinition_instances(VALUE self)
{
	SUComponentDefinitionRef definition = {DATA_PTR(self)};
	VALUE ary = rb_ary_new();
	FOREACH(
		SUComponentDefinitionGetNumInstances,
		SUComponentDefinitionGetInstances,
		SUComponentInstanceRef,
		definition,
		Sketchup_ComponentDefinition_instances_Iterator,
		ary);
	return ary;
}

static VALUE Sketchup_ComponentDefinition_count_instances(VALUE self)
{
	SUComponentDefinitionRef definition = {DATA_PTR(self)};
	size_t count = 0;
	SUComponentDefinitionGetNumInstances(definition, &count);
	return INT2NUM(count);
}

static VALUE Sketchup_ComponentDefinition_count_used_instances(VALUE self)
{
	SUComponentDefinitionRef definition = {DATA_PTR(self)};
	size_t count = 0;
	SUComponentDefinitionGetNumUsedInstances(definition, &count);
	return INT2NUM(count);
}

static VALUE Sketchup_ComponentDefinition_live_component(VALUE self)
{
	SUComponentDefinitionRef definition = {DATA_PTR(self)};
	bool output = false;
	SUComponentDefinitionIsLiveComponent(definition, &output);
	return output ? Qtrue : Qfalse;
}

static VALUE Sketchup_ComponentDefinition_internal(VALUE self)
{
	SUComponentDefinitionRef definition = {DATA_PTR(self)};
	bool is_internal = false;
	SUComponentDefinitionIsInternal(definition, &is_internal);
	return is_internal ? Qtrue : Qfalse;
}

static VALUE Sketchup_ComponentDefinition_entities(VALUE self)
{
	SUComponentDefinitionRef definition = {DATA_PTR(self)};
	SUEntitiesRef entities;
	SUComponentDefinitionGetEntities(definition, &entities);
	return Data_Wrap_Struct(rb_path2class(SKETCHUP_ENTITIES), 0, 0, entities.ptr);
}

static VALUE Sketchup_ComponentDefinition_path(VALUE self)
{
	SUComponentDefinitionRef definition = {DATA_PTR(self)};
	VALUE output;
	GETSTRING(SUComponentDefinitionGetPath, definition, output);
	return output;
}

static VALUE Sketchup_ComponentDefinition_guid(VALUE self)
{
	SUComponentDefinitionRef definition = {DATA_PTR(self)};
	VALUE output;
	GETSTRING(SUComponentDefinitionGetGuid, definition, output);
	return output;
}

static VALUE Sketchup_ComponentDefinition_group(VALUE self)
{
	SUComponentDefinitionRef definition = {DATA_PTR(self)};
	enum SUComponentType type = SUComponentType_Normal;
	SUComponentDefinitionGetType(definition, &type);
	return type == SUComponentType_Group ? Qtrue : Qfalse;
}

static VALUE Sketchup_ComponentDefinition_Get_description(VALUE self)
{
	SUComponentDefinitionRef definition = {DATA_PTR(self)};
	VALUE output;
	GETSTRING(SUComponentDefinitionGetDescription, definition, output);
	return output;
}

static VALUE Sketchup_ComponentDefinition_Set_description(VALUE self, VALUE description)
{
	SUComponentDefinitionRef definition = {DATA_PTR(self)};
	SUComponentDefinitionSetDescription(definition, StringValuePtr(description));
	return description;
}

static VALUE Sketchup_ComponentDefinition_image(VALUE self)
{
	SUComponentDefinitionRef definition = {DATA_PTR(self)};
	SUImageRef image = SUImageFromDrawingElement(SUComponentDefinitionToDrawingElement(definition));
	return SUIsValid(image) ? Qtrue : Qfalse;
}

static VALUE Sketchup_ComponentDefinition_behavior(VALUE self)
{
	SUComponentDefinitionRef definition = {DATA_PTR(self)};
	struct SUComponentBehavior behavior;
	SUComponentDefinitionGetBehavior(definition, &behavior);
	return Data_Wrap_Struct(rb_path2class(SKETCHUP_BEHAVIOR), 0, 0, &behavior);
}

static VALUE Sketchup_ComponentDefinition_save_as(int argc, VALUE* argv, VALUE self)
{
	VALUE path;
	VALUE version;
	rb_scan_args(argc, argv, "11", &path, &version);

	SUComponentDefinitionRef definition = {DATA_PTR(self)};
	enum SUResult result = SUComponentDefinitionSaveToFile(definition, StringValuePtr(path), NIL_P(version) ? SUModelVersion_Current : NUM2INT(version));
	if (result != SU_ERROR_NONE)
		return Qnil;
	return Qtrue;
}

VALUE ComponentDefinition_Init(VALUE Sketchup, VALUE Sketchup_DrawingElement)
{
	VALUE Sketchup_ComponentDefinition = rb_define_class_under(Sketchup, COMPONENTDEFINITION, Sketchup_DrawingElement);
	rb_define_method(Sketchup_ComponentDefinition, "name", Sketchup_ComponentDefinition_Get_name, 0);
	rb_define_method(Sketchup_ComponentDefinition, "name=", Sketchup_ComponentDefinition_Set_name, 1);
	rb_define_method(Sketchup_ComponentDefinition, "==", Sketchup_ComponentDefinition_Object_equ, 1);
	rb_define_method(Sketchup_ComponentDefinition, "<=>", Sketchup_ComponentDefinition_Name_equ, 1);
	rb_define_method(Sketchup_ComponentDefinition, "instances", Sketchup_ComponentDefinition_instances, 0);
	rb_define_method(Sketchup_ComponentDefinition, "count_instances", Sketchup_ComponentDefinition_count_instances, 0);
	rb_define_method(Sketchup_ComponentDefinition, "count_used_instances", Sketchup_ComponentDefinition_count_used_instances, 0);
	rb_define_method(Sketchup_ComponentDefinition, "live_component?", Sketchup_ComponentDefinition_live_component, 0);
	rb_define_method(Sketchup_ComponentDefinition, "internal?", Sketchup_ComponentDefinition_internal, 0);
	rb_define_method(Sketchup_ComponentDefinition, "entities", Sketchup_ComponentDefinition_entities, 0);
	rb_define_method(Sketchup_ComponentDefinition, "path", Sketchup_ComponentDefinition_path, 0);
	rb_define_method(Sketchup_ComponentDefinition, "guid", Sketchup_ComponentDefinition_guid, 0);
	rb_define_method(Sketchup_ComponentDefinition, "group?", Sketchup_ComponentDefinition_group, 0);
	rb_define_method(Sketchup_ComponentDefinition, "description", Sketchup_ComponentDefinition_Get_description, 0);
	rb_define_method(Sketchup_ComponentDefinition, "description=", Sketchup_ComponentDefinition_Set_description, 1);
	rb_define_method(Sketchup_ComponentDefinition, "image?", Sketchup_ComponentDefinition_image, 0);
	rb_define_method(Sketchup_ComponentDefinition, "behavior", Sketchup_ComponentDefinition_behavior, 0);
	rb_define_method(Sketchup_ComponentDefinition, "save_as", Sketchup_ComponentDefinition_save_as, -1);
	return Sketchup_ComponentDefinition;
}