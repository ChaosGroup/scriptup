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

// Sketchup::OptionsProvider: a named store reached through Model#options.
#define PROVIDER(self) SU_REF(SUOptionsProviderRef, self)
DEFINE_STRING_LIST(provider_key_list, SUOptionsProviderRef, SUOptionsProviderGetNumKeys, SUOptionsProviderGetKeys)
DEFINE_STRING_GETTER(provider_name, SUOptionsProviderRef, SUOptionsProviderGetName)
static enum SUResult provider_get(void* ptr, const char* key, SUTypedValueRef* value) { return SUOptionsProviderGetValue((SUOptionsProviderRef){ptr}, key, value); }
static enum SUResult provider_put(void* ptr, const char* key, SUTypedValueRef value) { return SUOptionsProviderSetValue((SUOptionsProviderRef){ptr}, key, value); }
static VALUE provider_keys(VALUE self) { return provider_key_list(PROVIDER(self)); }
static VALUE provider_length(VALUE self) { return LONG2NUM(RARRAY_LEN(provider_keys(self))); }
static VALUE provider_aset(VALUE self, VALUE key, VALUE value) { return store_set(self, key, value, provider_get, provider_put); }
static VALUE provider_has_key_p(VALUE self, VALUE key) { return rb_ary_includes(provider_keys(self), StringValue(key)); }
static VALUE provider_each_key(VALUE self) { su_yield_each(self, provider_keys(self)); return Qnil; }

static VALUE provider_aref(VALUE self, VALUE key)
{
	if (!RB_TYPE_P(key, T_STRING))
		key = rb_ary_entry(provider_keys(self), NUM2LONG(key));
	return NIL_P(key) ? Qnil : store_value(self, key, provider_get);
}

static VALUE provider_each(VALUE self)
{
	VALUE keys = provider_keys(self);
	for (long i = 0; i < RARRAY_LEN(keys); i++)
		rb_yield_values(2, RARRAY_AREF(keys, i), provider_aref(self, RARRAY_AREF(keys, i)));
	return Qnil;
}

static VALUE provider_each_value(VALUE self)
{
	VALUE keys = provider_keys(self);
	for (long i = 0; i < RARRAY_LEN(keys); i++)
		rb_yield(provider_aref(self, RARRAY_AREF(keys, i)));
	return Qnil;
}

// Sketchup::OptionsManager
#define MANAGER(self) SU_REF(SUOptionsManagerRef, self)
DEFINE_STRING_LIST(manager_key_list, SUOptionsManagerRef, SUOptionsManagerGetNumOptionsProviders, SUOptionsManagerGetOptionsProviderNames)
static VALUE manager_keys(VALUE self) { return manager_key_list(MANAGER(self)); }
static VALUE manager_length(VALUE self) { return LONG2NUM(RARRAY_LEN(manager_keys(self))); }

static VALUE manager_aref(VALUE self, VALUE key)
{
	if (!RB_TYPE_P(key, T_STRING))
		key = rb_ary_entry(manager_keys(self), NUM2LONG(key));
	if (NIL_P(key))
		return Qnil;
	SUOptionsProviderRef provider = SU_INVALID;
	SUOptionsManagerGetOptionsProviderByName(MANAGER(self), StringValueCStr(key), &provider);
	return su_wrap_or_nil(cOptionsProvider, provider.ptr);
}

static VALUE manager_each(VALUE self)
{
	VALUE keys = manager_keys(self);
	for (long i = 0; i < RARRAY_LEN(keys); i++)
		rb_yield(manager_aref(self, RARRAY_AREF(keys, i)));
	return Qnil;
}

static VALUE model_options(VALUE model)
{
	SUOptionsManagerRef manager = SU_INVALID;
	SUModelGetOptionsManager(SU_REF(SUModelRef, model), &manager);
	return su_wrap_or_nil(cOptionsManager, manager.ptr);
}

void Init_options_store(void)
{
	rb_include_module(cOptionsProvider, rb_mEnumerable);
	rb_define_method(cOptionsProvider, "name", provider_name, 0);
	rb_define_method(cOptionsProvider, "keys", provider_keys, 0);
	rb_define_method(cOptionsProvider, "[]", provider_aref, 1);
	rb_define_method(cOptionsProvider, "[]=", provider_aset, 2);
	rb_define_method(cOptionsProvider, "has_key?", provider_has_key_p, 1);
	rb_define_method(cOptionsProvider, "key?", provider_has_key_p, 1);
	rb_define_method(cOptionsProvider, "each", provider_each, 0);
	rb_define_method(cOptionsProvider, "each_pair", provider_each, 0);
	rb_define_method(cOptionsProvider, "each_key", provider_each_key, 0);
	rb_define_method(cOptionsProvider, "each_value", provider_each_value, 0);
	rb_define_method(cOptionsProvider, "count", provider_length, 0);
	rb_define_method(cOptionsProvider, "length", provider_length, 0);
	rb_define_method(cOptionsProvider, "size", provider_length, 0);
	rb_include_module(cOptionsManager, rb_mEnumerable);
	rb_define_method(cOptionsManager, "keys", manager_keys, 0);
	rb_define_method(cOptionsManager, "[]", manager_aref, 1);
	rb_define_method(cOptionsManager, "each", manager_each, 0);
	rb_define_method(cOptionsManager, "count", manager_length, 0);
	rb_define_method(cOptionsManager, "length", manager_length, 0);
	rb_define_method(cOptionsManager, "size", manager_length, 0);
	rb_define_method(cModel, "options", model_options, 0);
	renderingoptions_define("rendering_options");
	shadowinfo_define("shadow_info");
}
