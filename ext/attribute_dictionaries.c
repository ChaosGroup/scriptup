#include <scriptup.h>

// Wraps either a model or an entity; models are flagged with a hidden ivar.
#define OWNER_IS_MODEL "__owner_is_model"

DEFINE_REF_LIST(model_dictionaries, SUModelRef, SUModelGetNumAttributeDictionaries,
	SUModelGetAttributeDictionaries, SUAttributeDictionaryRef, cAttributeDictionary)
DEFINE_REF_LIST(entity_dictionaries, SUEntityRef, SUEntityGetNumAttributeDictionaries,
	SUEntityGetAttributeDictionaries, SUAttributeDictionaryRef, cAttributeDictionary)

VALUE su_model_attribute_dictionaries(SUModelRef model)
{
	VALUE obj = su_wrap(cAttributeDictionaries, model.ptr);
	rb_ivar_set(obj, (rb_intern)(OWNER_IS_MODEL), Qtrue);
	return obj;
}

static VALUE attributedictionaries_to_a(VALUE self)
{
	if (RTEST(rb_attr_get(self, (rb_intern)(OWNER_IS_MODEL))))
		return model_dictionaries(SU_REF(SUModelRef, self));
	return entity_dictionaries(SU_REF(SUEntityRef, self));
}

static VALUE attributedictionaries_aref(VALUE self, VALUE name)
{
	return su_find_by(attributedictionaries_to_a(self), "name", name);
}

static VALUE attributedictionaries_each(VALUE self)
{
	return su_yield_each(self, attributedictionaries_to_a(self));
}

static VALUE attributedictionaries_length(VALUE self)
{
	return LONG2NUM(RARRAY_LEN(attributedictionaries_to_a(self)));
}

void Init_attribute_dictionaries(void)
{
	rb_include_module(cAttributeDictionaries, rb_mEnumerable);
	rb_define_method(cAttributeDictionaries, "[]", attributedictionaries_aref, 1);
	rb_define_method(cAttributeDictionaries, "each", attributedictionaries_each, 0);
	rb_define_method(cAttributeDictionaries, "length", attributedictionaries_length, 0);
	rb_define_method(cAttributeDictionaries, "size", attributedictionaries_length, 0);
}
