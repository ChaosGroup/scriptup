#include <stdbool.h>
#include <attribute_dictionary.h>
#include <typed_value.h>
#include <utils.h>
#include <SketchUpAPI/sketchup.h>

static VALUE Sketchup_AttributeDictionary_get(VALUE self, VALUE key)
{
	SUAttributeDictionaryRef dictionary = {DATA_PTR(self)};
	SUTypedValueRef typedValue = SU_INVALID;
	SUTypedValueCreate(&typedValue);
	enum SUResult result = SUAttributeDictionaryGetValue(dictionary, StringValuePtr(key), &typedValue);
	if (result != SU_ERROR_NONE)
	{
		SUTypedValueRelease(&typedValue);
		return Qnil;
	}
	VALUE output = TypedValue_get_value_type(&typedValue);
	SUTypedValueRelease(&typedValue);
	return output;
}

static VALUE Sketchup_AttributeDictionary_set(VALUE self, VALUE key, VALUE value)
{
	SUAttributeDictionaryRef dictionary = {DATA_PTR(self)};
	SUTypedValueRef typedValue = SU_INVALID;
	SUTypedValueCreate(&typedValue);
	TypedValue_set_value_type(&typedValue, value);
	enum SUResult result = SUAttributeDictionarySetValue(dictionary, StringValuePtr(key), typedValue);
	SUTypedValueRelease(&typedValue);
	if (result != SU_ERROR_NONE)
		return Qnil;
	return TypedValue_get_value_type(&typedValue);
}

void Sketchup_AttributeDictionary_Iterator(SUStringRef dictionary_key, SUAttributeDictionaryRef dictionary)
{
	VALUE key;
	GETUTF8FROMSTRING(dictionary_key, key);
	SUTypedValueRef value = SU_INVALID;
	SUTypedValueCreate(&value);
	SUAttributeDictionaryGetValue(dictionary, StringValuePtr(key), &value);
	VALUE output = TypedValue_get_value_type(&value);
	SUTypedValueRelease(&value);
	rb_yield_values(2, key, output);
}

static VALUE Sketchup_AttributeDictionary_each(VALUE self)
{

	SUAttributeDictionaryRef dictionary = {DATA_PTR(self)};
	FOREACHSTRING(SUAttributeDictionaryGetNumKeys, SUAttributeDictionaryGetKeys, SUStringRef, dictionary, Sketchup_AttributeDictionary_Iterator, dictionary);
	return self;
}

void Sketchup_AttributeDictionary_each_key_Iterator(SUStringRef dictionary_key, void* _)
{
	VALUE key;
	GETUTF8FROMSTRING(dictionary_key, key);
	rb_yield(key);
}

static VALUE Sketchup_AttributeDictionary_each_key(VALUE self)
{
	SUAttributeDictionaryRef dictionary = {DATA_PTR(self)};
	FOREACHSTRING(SUAttributeDictionaryGetNumKeys, SUAttributeDictionaryGetKeys, SUStringRef, dictionary, Sketchup_AttributeDictionary_each_key_Iterator, 0);
	return self;
}

static VALUE Sketchup_AttributeDictionary_name(VALUE self)
{
	VALUE output;
	SUAttributeDictionaryRef dictionary = {DATA_PTR(self)};
	GETSTRING(SUAttributeDictionaryGetName, dictionary, output)
	return output;
}

static VALUE Sketchup_AttributeDictionary_length(VALUE self)
{
	SUAttributeDictionaryRef dictionary = {DATA_PTR(self)};
	size_t count;
	SUAttributeDictionaryGetNumKeys(dictionary, &count);	
	return INT2FIX(count);
}

void Sketchup_AttributeDictionary_keys_Iterator(SUStringRef dictionary_key, VALUE ary)
{
	VALUE key;
	GETUTF8FROMSTRING(dictionary_key, key);
	rb_ary_push(ary, key);
}

static VALUE Sketchup_AttributeDictionary_keys(VALUE self)
{
	SUAttributeDictionaryRef dictionary = {DATA_PTR(self)};
	VALUE ary = rb_ary_new();
	FOREACHSTRING(SUAttributeDictionaryGetNumKeys, SUAttributeDictionaryGetKeys, SUStringRef, dictionary, Sketchup_AttributeDictionary_keys_Iterator, ary);
	return ary;
}

struct AttributeDictionary_Struct
{
	VALUE ary;
	SUAttributeDictionaryRef dictionary;
};

void Sketchup_AttributeDictionary_values_Iterator(SUStringRef dictionary_key, struct AttributeDictionary_Struct attributeDictionary_struct)
{
	VALUE key;
	GETUTF8FROMSTRING(dictionary_key, key);
	SUTypedValueRef value = SU_INVALID;
	SUTypedValueCreate(&value);
	SUAttributeDictionaryGetValue(attributeDictionary_struct.dictionary, StringValuePtr(key), &value);
	VALUE output = TypedValue_get_value_type(&value);
	SUTypedValueRelease(&value);
	rb_ary_push(attributeDictionary_struct.ary, output);
}

static VALUE Sketchup_AttributeDictionary_values(VALUE self)
{
	SUAttributeDictionaryRef dictionary = {DATA_PTR(self)};
	VALUE ary = rb_ary_new();
	struct AttributeDictionary_Struct values_struct = {ary, dictionary};
	FOREACHSTRING(SUAttributeDictionaryGetNumKeys, SUAttributeDictionaryGetKeys, SUStringRef, dictionary, Sketchup_AttributeDictionary_values_Iterator, values_struct);
	return ary;
}

static VALUE Sketchup_AttributeDictionary_delete_key(VALUE self, VALUE key)
{
	SUAttributeDictionaryRef dictionary = {DATA_PTR(self)};
	SUTypedValueRef value = SU_INVALID;
	SUTypedValueCreate(&value);
	SUAttributeDictionaryGetValue(dictionary, StringValuePtr(key), &value);
	VALUE output = TypedValue_get_value_type(&value);
	SUTypedValueRelease(&value);
	// ToDo:: delete the key and its value from the attribute dictionary
	return output;
}

VALUE AttributeDictionary_Init(VALUE Sketchup, VALUE Sketchup_Entity)
{
	VALUE Sketchup_AttributeDictionary = rb_define_class_under(Sketchup, ATTRIBUTEDICTIONARY, Sketchup_Entity);
	rb_include_module(Sketchup_AttributeDictionary, rb_mEnumerable);
	rb_define_method(Sketchup_AttributeDictionary, "[]", Sketchup_AttributeDictionary_get, 1);
	rb_define_method(Sketchup_AttributeDictionary, "[]=", Sketchup_AttributeDictionary_set, 2);
	rb_define_method(Sketchup_AttributeDictionary, "each", Sketchup_AttributeDictionary_each, 0);
	rb_define_method(Sketchup_AttributeDictionary, "each_pair", Sketchup_AttributeDictionary_each, 0);
	rb_define_method(Sketchup_AttributeDictionary, "each_key", Sketchup_AttributeDictionary_each_key, 0);
	rb_define_method(Sketchup_AttributeDictionary, "name", Sketchup_AttributeDictionary_name, 0);
	rb_define_method(Sketchup_AttributeDictionary, "length", Sketchup_AttributeDictionary_length, 0);
	rb_define_method(Sketchup_AttributeDictionary, "size", Sketchup_AttributeDictionary_length, 0);
	rb_define_method(Sketchup_AttributeDictionary, "keys", Sketchup_AttributeDictionary_keys, 0);
	rb_define_method(Sketchup_AttributeDictionary, "values", Sketchup_AttributeDictionary_values, 0);
	rb_define_method(Sketchup_AttributeDictionary, "delete_key", Sketchup_AttributeDictionary_delete_key, 1);
	return Sketchup_AttributeDictionary;
}