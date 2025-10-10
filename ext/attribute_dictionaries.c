#include <stdbool.h>
#include <attribute_dictionaries.h>
#include <utils.h>
#include <SketchUpAPI/sketchup.h>

struct AttributeDictionaryStringIterationContext
{
	VALUE key;
	SUAttributeDictionaryRef dictionary;
};

void Sketchup_AttributeDictionaries_Get_By_String_Iterator(SUAttributeDictionaryRef dictionary, struct AttributeDictionaryStringIterationContext* dictionary_struct)
{
	VALUE output;
	GETSTRING(SUAttributeDictionaryGetName, dictionary, output);
	if (strcmp(StringValuePtr(output), StringValuePtr(dictionary_struct->key)) == 0)
		dictionary_struct->dictionary = dictionary;
}

static VALUE Sketchup_AttributeDictionaries_get(VALUE self, VALUE key)
{
	SUAttributeDictionaryRef dictionary = SU_INVALID;
	if (rb_ivar_defined(self, rb_intern("@is_model")) == Qtrue && RTEST(rb_iv_get(self, "@is_model")))
	{
		SUModelRef model = {DATA_PTR(self)};
		enum SUResult result = SUModelGetAttributeDictionary(model, StringValuePtr(key), &dictionary);
		if (result != SU_ERROR_NONE)
			return Qnil;
	}
	else
	{
		SUEntityRef entity = {DATA_PTR(self)};
		enum SUResult result = SUEntityGetAttributeDictionary(entity, StringValuePtr(key), &dictionary);
		if (result != SU_ERROR_NONE)
			return Qnil;
	}
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

static VALUE Sketchup_AttributeDictionaries_delete(VALUE self, VALUE dictionary)
{
	if (rb_ivar_defined(self, rb_intern("@is_model")) == Qtrue && RTEST(rb_iv_get(self, "@is_model")))
	{
		SUModelRef model = {DATA_PTR(self)};
		if (rb_type(dictionary) == T_STRING)
		{
			struct AttributeDictionaryStringIterationContext dictionary_struct = {dictionary, SU_INVALID};
			FOREACH(SUModelGetNumAttributeDictionaries, SUModelGetAttributeDictionaries, SUAttributeDictionaryRef, model, Sketchup_AttributeDictionaries_Get_By_String_Iterator, &dictionary_struct);
			if (SUIsInvalid(dictionary_struct.dictionary))
				return Qnil;
			SUAttributeDictionaryRelease(dictionary_struct.dictionary.ptr);
			size_t ad_count = 0;
			size_t ad_len = 0;
			SUModelGetNumAttributeDictionaries(model, &ad_len);
			SUAttributeDictionaryRef* dictionaries;
			SUModelGetAttributeDictionaries(model, ad_len, dictionaries, &ad_count);
			return Data_Wrap_Struct(rb_path2class(SKETCHUP_ATTRIBUTEDICTIONARIES), 0, 0, dictionaries);
		}
		else if (rb_obj_is_kind_of(dictionary, rb_path2class(SKETCHUP_ATTRIBUTEDICTIONARY)))
		{
			SUAttributeDictionaryRef dictionary_ref = {DATA_PTR(dictionary)};
			SUAttributeDictionaryRelease(dictionary_ref.ptr);
			size_t ad_count = 0;
			size_t ad_len = 0;
			SUModelGetNumAttributeDictionaries(model, &ad_len);
			SUAttributeDictionaryRef* dictionaries;
			SUModelGetAttributeDictionaries(model, ad_len, dictionaries, &ad_count);
			return Data_Wrap_Struct(rb_path2class(SKETCHUP_ATTRIBUTEDICTIONARIES), 0, 0, dictionaries);
		}
		else
			rb_raise(rb_eArgError, "Wrong argument type.  Expected String or AttributeDictionary");
	}
	else
	{
		SUEntityRef entity = {DATA_PTR(self)};
		if (rb_type(dictionary) == T_STRING)
		{
			struct AttributeDictionaryStringIterationContext dictionary_struct = {dictionary, SU_INVALID};
			FOREACH(SUEntityGetNumAttributeDictionaries, SUEntityGetAttributeDictionaries, SUAttributeDictionaryRef, entity, Sketchup_AttributeDictionaries_Get_By_String_Iterator, &dictionary_struct);
			if (SUIsInvalid(dictionary_struct.dictionary))
				return Qnil;
			SUAttributeDictionaryRelease(dictionary_struct.dictionary.ptr);
			size_t ad_count = 0;
			size_t ad_len = 0;
			SUEntityGetNumAttributeDictionaries(entity, &ad_len);
			SUAttributeDictionaryRef* dictionaries;
			SUEntityGetAttributeDictionaries(entity, ad_len, dictionaries, &ad_count);
			return Data_Wrap_Struct(rb_path2class(SKETCHUP_ATTRIBUTEDICTIONARIES), 0, 0, dictionaries);
		}
		else if (rb_obj_is_kind_of(dictionary, rb_path2class(SKETCHUP_ATTRIBUTEDICTIONARY)))
		{
			SUAttributeDictionaryRef dictionary_ref = {DATA_PTR(self)};
			SUAttributeDictionaryRelease(dictionary_ref.ptr);
			size_t ad_count = 0;
			size_t ad_len = 0;
			SUEntityGetNumAttributeDictionaries(entity, &ad_len);
			SUAttributeDictionaryRef* dictionaries;
			SUEntityGetAttributeDictionaries(entity, ad_len, dictionaries, &ad_count);
			return Data_Wrap_Struct(rb_path2class(SKETCHUP_ATTRIBUTEDICTIONARIES), 0, 0, dictionaries);
		}
		else
			rb_raise(rb_eArgError, "Wrong argument type.  Expected String or AttributeDictionary");
	}
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
	rb_define_method(Sketchup_AttributeDictionaries, "delete", Sketchup_AttributeDictionaries_delete, 1);
	return Sketchup_AttributeDictionaries;
}