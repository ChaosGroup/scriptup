#include <stdbool.h>
#include <definition_list.h>
#include <utils.h>
#include <SketchUpAPI/sketchup.h>

void Sketchup_ComponentDefinition_Iterator(SUComponentDefinitionRef definition, void* _)
{
	rb_yield(Data_Wrap_Struct(rb_path2class(SKETCHUP_COMPONENTDEFINITION), 0, 0, definition.ptr));
}

void Sketchup_Group_Iterator(SUComponentDefinitionRef group, void* _)
{
	rb_yield(Data_Wrap_Struct(rb_path2class(SKETCHUP_COMPONENTDEFINITION), 0, 0, group.ptr));
}

static VALUE Sketchup_DefinitionList_each(VALUE self)
{
	SUModelRef model = {DATA_PTR(self)};
	FOREACH(SUModelGetNumComponentDefinitions, SUModelGetComponentDefinitions, SUComponentDefinitionRef, model, Sketchup_ComponentDefinition_Iterator, 0);
	FOREACH(SUModelGetNumGroupDefinitions, SUModelGetGroupDefinitions, SUComponentDefinitionRef, model, Sketchup_Group_Iterator, 0);
	return self;
}

static VALUE Sketchup_DefinitionList_count(VALUE self)
{
	SUModelRef model = {DATA_PTR(self)};
	size_t cd_count = 0;
	size_t gd_count = 0;
	SUModelGetNumComponentDefinitions(model, &cd_count);
	SUModelGetNumGroupDefinitions(model, &gd_count);
	return INT2NUM(cd_count + gd_count);
}

static VALUE Sketchup_DefinitionList_load(VALUE self, VALUE path)
{
	SUModelRef model = {DATA_PTR(self)};
	SUComponentDefinitionRef definition = SU_INVALID;
	enum SUModelLoadStatus status;
	enum SUResult result = SUModelLoadDefinitionWithStatus(model, StringValuePtr(path), &definition, &status);
	if (result != SU_ERROR_NONE)
		rb_raise(rb_eStandardError, "Unsuccessful loading of the definition!");
	return Data_Wrap_Struct(rb_path2class(SKETCHUP_COMPONENTDEFINITION), 0, 0, definition.ptr);
}

struct DefinitionStringIterationContext
{
	VALUE key;
	SUComponentDefinitionRef definition;
};

void Sketchup_DefinitionList_Get_By_String_Iterator(SUComponentDefinitionRef definition, struct DefinitionStringIterationContext* definition_struct)
{
	VALUE definition_name;
	VALUE definition_guid;
	GETSTRING(SUComponentDefinitionGetName, definition, definition_name);
	GETSTRING(SUComponentDefinitionGetGuid, definition, definition_guid);
	if (strcmp(StringValuePtr(definition_name), StringValuePtr(definition_struct->key)) == 0 ||
		strcmp(StringValuePtr(definition_guid), StringValuePtr(definition_struct->key)) == 0)
		definition_struct->definition = definition;
}

struct DefinitionIndexIterationContext
{
	int targetIndex;
	int currentIndex;
	SUComponentDefinitionRef definition;
};

void Sketchup_DefinitionList_Get_By_Index_Iterator(SUComponentDefinitionRef definition, struct DefinitionIndexIterationContext* definition_struct)
{
	if (definition_struct->targetIndex == definition_struct->currentIndex)
		definition_struct->definition = definition;
	definition_struct->currentIndex += 1;
}

static VALUE Sketchup_DefinitionList_get(VALUE self, VALUE key)
{
	SUModelRef model = {DATA_PTR(self)};
	if (rb_type(key) == T_STRING)
	{
		struct DefinitionStringIterationContext definition_struct = {key, SU_INVALID};
		FOREACH(
			SUModelGetNumComponentDefinitions,
			SUModelGetComponentDefinitions,
			SUComponentDefinitionRef,
			model,
			Sketchup_DefinitionList_Get_By_String_Iterator,
			&definition_struct);
		if (SUIsInvalid(definition_struct.definition))
			return Qnil;
		return Data_Wrap_Struct(rb_path2class(SKETCHUP_COMPONENTDEFINITION), 0, 0, definition_struct.definition.ptr);
	}
	else
	{
		struct DefinitionIndexIterationContext definition_struct = {NUM2INT(key), 0, SU_INVALID};
		FOREACH(
			SUModelGetNumComponentDefinitions,
			SUModelGetComponentDefinitions,
			SUComponentDefinitionRef,
			model,
			Sketchup_DefinitionList_Get_By_Index_Iterator,
			&definition_struct);
		if (SUIsInvalid(definition_struct.definition))
		{
			if (definition_struct.targetIndex >= definition_struct.currentIndex)
				rb_raise(rb_eIndexError, "index %d out of range [0, %d)", definition_struct.targetIndex, definition_struct.currentIndex);
		}
		return Data_Wrap_Struct(rb_path2class(SKETCHUP_COMPONENTDEFINITION), 0, 0, definition_struct.definition.ptr);
	}
}

static VALUE Sketchup_DefinitionList_add(VALUE self, VALUE name)
{
	SUModelRef model = {DATA_PTR(self)};
	SUComponentDefinitionRef definition = SU_INVALID;
	SUComponentDefinitionCreate(&definition);
	SUComponentDefinitionSetName(definition, StringValuePtr(name));
	SUModelAddComponentDefinitions(model, 1, &definition);
	return Data_Wrap_Struct(rb_path2class(SKETCHUP_COMPONENTDEFINITION), 0, 0, definition.ptr);
}

static VALUE Sketchup_DefinitionList_remove(VALUE self, VALUE rb_definition)
{
	if (!rb_obj_is_kind_of(rb_definition, rb_path2class(SKETCHUP_COMPONENTDEFINITION)))
		rb_raise(rb_eTypeError, "Wrong type of object given");
	SUModelRef model = {DATA_PTR(self)};
	SUComponentDefinitionRef definition = {DATA_PTR(rb_definition)};
	SUModelRemoveComponentDefinitions(model, 1, &definition);
	return Qtrue;
}

VALUE DefinitionList_Init(VALUE Sketchup, VALUE Sketchup_Entity)
{
	VALUE Sketchup_DefinitionList = rb_define_class_under(Sketchup, DEFINITIONLIST, Sketchup_Entity);
	rb_include_module(Sketchup_DefinitionList, rb_mEnumerable);
	rb_define_method(Sketchup_DefinitionList, "each", Sketchup_DefinitionList_each, 0);
	rb_define_method(Sketchup_DefinitionList, "count", Sketchup_DefinitionList_count, 0);
	rb_define_method(Sketchup_DefinitionList, "size", Sketchup_DefinitionList_count, 0);
	rb_define_method(Sketchup_DefinitionList, "length", Sketchup_DefinitionList_count, 0);
	rb_define_method(Sketchup_DefinitionList, "load", Sketchup_DefinitionList_load, 1);
	rb_define_method(Sketchup_DefinitionList, "[]", Sketchup_DefinitionList_get, 1);
	rb_define_method(Sketchup_DefinitionList, "add", Sketchup_DefinitionList_add, 1);
	rb_define_method(Sketchup_DefinitionList, "remove", Sketchup_DefinitionList_remove, 1);
	return Sketchup_DefinitionList;
}