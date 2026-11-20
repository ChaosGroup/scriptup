#include <scriptup.h>

DEFINE_STRING_GETTER(attributedictionary_name, SUAttributeDictionaryRef, SUAttributeDictionaryGetName)
DEFINE_SIZE_GETTER(attributedictionary_length, SUAttributeDictionaryRef, SUAttributeDictionaryGetNumKeys)
DEFINE_STRING_LIST(attributedictionary_key_list, SUAttributeDictionaryRef, SUAttributeDictionaryGetNumKeys,
	SUAttributeDictionaryGetKeys)

static VALUE attributedictionary_value(SUAttributeDictionaryRef dictionary, VALUE key)
{
	SUTypedValueRef value = SU_INVALID;
	SUTypedValueCreate(&value);
	VALUE output = Qnil;
	if (SUAttributeDictionaryGetValue(dictionary, StringValueCStr(key), &value) == SU_ERROR_NONE)
		output = su_ruby_from_typed_value(value);
	SUTypedValueRelease(&value);
	return output;
}

static VALUE attributedictionary_keys(VALUE self)
{
	return attributedictionary_key_list(SU_REF(SUAttributeDictionaryRef, self));
}

static VALUE attributedictionary_values(VALUE self)
{
	SUAttributeDictionaryRef dictionary = SU_REF(SUAttributeDictionaryRef, self);
	VALUE keys = attributedictionary_key_list(dictionary);
	VALUE values = rb_ary_new_capa(RARRAY_LEN(keys));
	for (long i = 0; i < RARRAY_LEN(keys); i++)
		rb_ary_push(values, attributedictionary_value(dictionary, RARRAY_AREF(keys, i)));
	return values;
}

static VALUE attributedictionary_aref(VALUE self, VALUE key)
{
	return attributedictionary_value(SU_REF(SUAttributeDictionaryRef, self), key);
}

static VALUE attributedictionary_aset(VALUE self, VALUE key, VALUE value)
{
	SUTypedValueRef typed_value = SU_INVALID;
	SUTypedValueCreate(&typed_value);
	su_typed_value_from_ruby(typed_value, value);
	enum SUResult result = SUAttributeDictionarySetValue(SU_REF(SUAttributeDictionaryRef, self), StringValueCStr(key), typed_value);
	SUTypedValueRelease(&typed_value);
	return result == SU_ERROR_NONE ? value : Qnil;
}

static VALUE attributedictionary_each(VALUE self)
{
	VALUE keys = attributedictionary_keys(self);
	VALUE values = attributedictionary_values(self);
	for (long i = 0; i < RARRAY_LEN(keys); i++)
		rb_yield_values(2, RARRAY_AREF(keys, i), RARRAY_AREF(values, i));
	return self;
}

static VALUE attributedictionary_each_key(VALUE self)
{
	return su_yield_each(self, attributedictionary_keys(self));
}

static VALUE attributedictionary_delete_key(VALUE self, VALUE key)
{
	return attributedictionary_aref(self, key); // ToDo: delete the key once the C API supports it
}

void Init_attribute_dictionary(void)
{
	rb_include_module(cAttributeDictionary, rb_mEnumerable);
	rb_define_method(cAttributeDictionary, "[]", attributedictionary_aref, 1);
	rb_define_method(cAttributeDictionary, "[]=", attributedictionary_aset, 2);
	rb_define_method(cAttributeDictionary, "each", attributedictionary_each, 0);
	rb_define_method(cAttributeDictionary, "each_pair", attributedictionary_each, 0);
	rb_define_method(cAttributeDictionary, "each_key", attributedictionary_each_key, 0);
	rb_define_method(cAttributeDictionary, "name", attributedictionary_name, 0);
	rb_define_method(cAttributeDictionary, "length", attributedictionary_length, 0);
	rb_define_method(cAttributeDictionary, "size", attributedictionary_length, 0);
	rb_define_method(cAttributeDictionary, "keys", attributedictionary_keys, 0);
	rb_define_method(cAttributeDictionary, "values", attributedictionary_values, 0);
	rb_define_method(cAttributeDictionary, "delete_key", attributedictionary_delete_key, 1);
	rb_define_alias(cAttributeDictionary, "count", "size");
}
