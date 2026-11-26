#include <scriptup.h>

// RenderingOptions and ShadowInfo are both key/value stores of typed values.
static VALUE store_value(VALUE self, VALUE key, enum SUResult (*get)(void*, const char*, SUTypedValueRef*))
{
	SUTypedValueRef value = SU_INVALID;
	SUTypedValueCreate(&value);
	VALUE output = Qnil;
	if (get(DATA_PTR(self), StringValueCStr(key), &value) == SU_ERROR_NONE)
		output = su_ruby_from_typed_value(value);
	SUTypedValueRelease(&value);
	return output;
}

// Numbers are stored with the type the key already has (e.g. 40 for a double).
static VALUE store_set(VALUE self, VALUE key, VALUE object, enum SUResult (*get)(void*, const char*, SUTypedValueRef*),
	enum SUResult (*set)(void*, const char*, SUTypedValueRef))
{
	const char* name = StringValueCStr(key);
	SUTypedValueRef value = SU_INVALID;
	SUTypedValueCreate(&value);
	enum SUTypedValueType type = SUTypedValueType_Empty;
	if (get(DATA_PTR(self), name, &value) != SU_ERROR_NONE)
	{
		SUTypedValueRelease(&value);
		rb_raise(rb_eArgError, "invalid key: %s", name);
	}
	SUTypedValueGetType(value, &type);
	if (rb_obj_is_kind_of(object, rb_cNumeric) && (type == SUTypedValueType_Double || type == SUTypedValueType_Float))
		SUTypedValueSetDouble(value, NUM2DBL(object));
	else if (rb_obj_is_kind_of(object, rb_cNumeric) && type == SUTypedValueType_Short)
		SUTypedValueSetInt16(value, (int16_t)NUM2INT(object));
	else
		su_typed_value_from_ruby(value, object);
	enum SUResult result = set(DATA_PTR(self), name, value);
	SUTypedValueRelease(&value);
	if (result != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "cannot set %s", name);
	return object;
}

#define DEFINE_STORE(prefix, klass, RefType, NumFn, KeysFn, GetFn, SetFn, ModelFn) \
	DEFINE_STRING_LIST(prefix##_key_list, RefType, NumFn, KeysFn) \
	static enum SUResult prefix##_get(void* ptr, const char* key, SUTypedValueRef* value) { return GetFn((RefType){ptr}, key, value); } \
	static enum SUResult prefix##_put(void* ptr, const char* key, SUTypedValueRef value) { return SetFn((RefType){ptr}, key, value); } \
	static VALUE prefix##_keys(VALUE self) { return prefix##_key_list(SU_REF(RefType, self)); } \
	static VALUE prefix##_length(VALUE self) { return LONG2NUM(RARRAY_LEN(prefix##_keys(self))); } \
	static VALUE prefix##_aref(VALUE self, VALUE key) { return store_value(self, key, prefix##_get); } \
	static VALUE prefix##_aset(VALUE self, VALUE key, VALUE value) { return store_set(self, key, value, prefix##_get, prefix##_put); } \
	static VALUE prefix##_each_key(VALUE self) { su_yield_each(self, prefix##_keys(self)); return Qnil; } \
	static VALUE prefix##_each(VALUE self) \
	{ \
		VALUE keys = prefix##_keys(self); \
		for (long i = 0; i < RARRAY_LEN(keys); i++) \
			rb_yield_values(2, RARRAY_AREF(keys, i), prefix##_aref(self, RARRAY_AREF(keys, i))); \
		return Qnil; \
	} \
	static VALUE prefix##_model_store(VALUE model) \
	{ \
		RefType ref = SU_INVALID; \
		ModelFn(SU_REF(SUModelRef, model), &ref); \
		return su_wrap_or_nil(klass, ref.ptr); \
	} \
	static VALUE prefix##_active_store(void) \
	{ \
		SUModelRef model = Sketchup_get_active_model(); \
		if (SUIsInvalid(model)) \
			rb_raise(rb_eRuntimeError, "no active model"); \
		return prefix##_model_store(su_wrap(cModel, model.ptr)); \
	} \
	static VALUE prefix##_s_keys(VALUE klass) { return prefix##_keys(prefix##_active_store()); } \
	static VALUE prefix##_s_each_key(VALUE klass) { return prefix##_each_key(prefix##_active_store()); } \
	static void prefix##_define(const char* model_method) \
	{ \
		rb_include_module(klass, rb_mEnumerable); \
		rb_define_singleton_method(klass, "keys", prefix##_s_keys, 0); \
		rb_define_singleton_method(klass, "each_key", prefix##_s_each_key, 0); \
		rb_define_method(klass, "keys", prefix##_keys, 0); \
		rb_define_method(klass, "[]", prefix##_aref, 1); \
		rb_define_method(klass, "[]=", prefix##_aset, 2); \
		rb_define_method(klass, "each", prefix##_each, 0); \
		rb_define_method(klass, "each_pair", prefix##_each, 0); \
		rb_define_method(klass, "each_key", prefix##_each_key, 0); \
		rb_define_method(klass, "count", prefix##_length, 0); \
		rb_define_method(klass, "length", prefix##_length, 0); \
		rb_define_method(klass, "size", prefix##_length, 0); \
		rb_define_method(cModel, model_method, prefix##_model_store, 0); \
	}

DEFINE_STORE(renderingoptions, cRenderingOptions, SURenderingOptionsRef, SURenderingOptionsGetNumKeys,
	SURenderingOptionsGetKeys, SURenderingOptionsGetValue, SURenderingOptionsSetValue, SUModelGetRenderingOptions)
DEFINE_STORE(shadowinfo, cShadowInfo, SUShadowInfoRef, SUShadowInfoGetNumKeys,
	SUShadowInfoGetKeys, SUShadowInfoGetValue, SUShadowInfoSetValue, SUModelGetShadowInfo)

void Init_options_store(void)
{
	renderingoptions_define("rendering_options");
	shadowinfo_define("shadow_info");
}
