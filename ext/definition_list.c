#include <scriptup.h>

DEFINE_REF_LIST(component_definitions, SUModelRef, SUModelGetNumComponentDefinitions,
	SUModelGetComponentDefinitions, SUComponentDefinitionRef, cComponentDefinition)
DEFINE_REF_LIST(group_definitions, SUModelRef, SUModelGetNumGroupDefinitions,
	SUModelGetGroupDefinitions, SUComponentDefinitionRef, cComponentDefinition)

static VALUE definitionlist_to_a(VALUE self)
{
	SUModelRef model = SU_REF(SUModelRef, self);
	return rb_ary_concat(component_definitions(model), group_definitions(model));
}

static VALUE definitionlist_each(VALUE self)
{
	return su_yield_each(self, definitionlist_to_a(self));
}

static VALUE definitionlist_length(VALUE self)
{
	SUModelRef model = SU_REF(SUModelRef, self);
	size_t components = 0;
	size_t groups = 0;
	SUModelGetNumComponentDefinitions(model, &components);
	SUModelGetNumGroupDefinitions(model, &groups);
	return SIZET2NUM(components + groups);
}

static VALUE definitionlist_aref(VALUE self, VALUE key)
{
	VALUE definitions = definitionlist_to_a(self);
	if (!RB_TYPE_P(key, T_STRING))
		return su_fetch(definitions, key);
	VALUE found = su_find_by(definitions, "name", key);
	return NIL_P(found) ? su_find_by(definitions, "guid", key) : found;
}

static VALUE definitionlist_load(VALUE self, VALUE path)
{
	SUComponentDefinitionRef definition = SU_INVALID;
	enum SUModelLoadStatus status;
	if (SUModelLoadDefinitionWithStatus(SU_REF(SUModelRef, self), StringValueCStr(path), &definition, &status) != SU_ERROR_NONE)
		rb_raise(rb_eStandardError, "Unsuccessful loading of the definition!");
	return su_wrap(cComponentDefinition, definition.ptr);
}

static VALUE definitionlist_add(VALUE self, VALUE name)
{
	const char* name_ptr = StringValueCStr(name);
	SUComponentDefinitionRef definition = SU_INVALID;
	SUComponentDefinitionCreate(&definition);
	SUComponentDefinitionSetName(definition, name_ptr);
	SUModelAddComponentDefinitions(SU_REF(SUModelRef, self), 1, &definition);
	return su_wrap(cComponentDefinition, definition.ptr);
}

static VALUE definitionlist_remove(VALUE self, VALUE definition)
{
	SUComponentDefinitionRef ref = {su_unwrap(definition, cComponentDefinition)};
	SUModelRemoveComponentDefinitions(SU_REF(SUModelRef, self), 1, &ref);
	return Qtrue;
}

void Init_definition_list(void)
{
	rb_include_module(cDefinitionList, rb_mEnumerable);
	rb_define_method(cDefinitionList, "each", definitionlist_each, 0);
	rb_define_method(cDefinitionList, "count", definitionlist_length, 0);
	rb_define_method(cDefinitionList, "size", definitionlist_length, 0);
	rb_define_method(cDefinitionList, "length", definitionlist_length, 0);
	rb_define_method(cDefinitionList, "load", definitionlist_load, 1);
	rb_define_method(cDefinitionList, "[]", definitionlist_aref, 1);
	rb_define_method(cDefinitionList, "add", definitionlist_add, 1);
	rb_define_method(cDefinitionList, "remove", definitionlist_remove, 1);
}
