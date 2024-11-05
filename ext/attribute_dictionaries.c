#include <stdbool.h>
#include <attribute_dictionaries.h>
#include <utils.h>
#include <SketchUpAPI/sketchup.h>

static VALUE Sketchup_AttributeDictionaries_get(VALUE self, VALUE key)
{
	SUModelRef model = {DATA_PTR(self)};
	SUAttributeDictionaryRef dictionary = SU_INVALID;
	enum SUResult result = SUModelGetAttributeDictionary(model, StringValuePtr(key), &dictionary);
	if (result != SU_ERROR_NONE)
		return Qnil;
	return Data_Wrap_Struct(rb_path2class(SKETCHUP_ATTRIBUTEDICTIONARY), 0, 0, dictionary.ptr);
}

void Sketchup_AttributeDictionaries_Iterator(SUAttributeDictionaryRef group, void* _)
{
	rb_yield(Data_Wrap_Struct(rb_path2class(SKETCHUP_ATTRIBUTEDICTIONARY), 0, 0, group.ptr));
}

static VALUE Sketchup_AttributeDictionaries_each(VALUE self)
{
	SUModelRef model = {DATA_PTR(self)};
	FOREACH(SUModelGetNumAttributeDictionaries, SUModelGetAttributeDictionaries, SUAttributeDictionaryRef, model, Sketchup_AttributeDictionaries_Iterator, 0);
	return self;
}

static VALUE Sketchup_AttributeDictionaries_length(VALUE self)
{
	SUModelRef model = {DATA_PTR(self)};
	size_t count;
	SUModelGetNumAttributeDictionaries(model, &count);
	return ULL2NUM(count);
}

VALUE AttributeDictionaries_Init(VALUE Sketchup, VALUE Sketchup_Entity)
{
	VALUE Sketchup_AttributeDictionaries = rb_define_class_under(Sketchup, ATTRIBUTEDICTIONARIES, Sketchup_Entity);
	rb_undef_alloc_func(Sketchup_AttributeDictionaries);
	rb_include_module(Sketchup_AttributeDictionaries, rb_mEnumerable);
	rb_define_method(Sketchup_AttributeDictionaries, "[]", Sketchup_AttributeDictionaries_get, 1);
	rb_define_method(Sketchup_AttributeDictionaries, "each", Sketchup_AttributeDictionaries_each, 0);
	rb_define_method(Sketchup_AttributeDictionaries, "length", Sketchup_AttributeDictionaries_length, 0);
	rb_define_method(Sketchup_AttributeDictionaries, "size", Sketchup_AttributeDictionaries_length, 0);
	return Sketchup_AttributeDictionaries;
}