#include <scriptup.h>

#define CLASSIFICATIONS(self) SU_REF(SUClassificationsRef, self)

DEFINE_STRING_GETTER(classificationschema_name, SUSchemaRef, SUSchemaGetSchemaName)
DEFINE_REF_LIST(classifications_list, SUClassificationsRef, SUClassificationsGetNumSchemas, SUClassificationsGetSchemas, SUSchemaRef, cClassificationSchema)

static VALUE classificationschema_cmp(VALUE self, VALUE other)
{
	if (!rb_obj_is_kind_of(other, cClassificationSchema))
		return Qnil;
	return INT2FIX(rb_str_cmp(classificationschema_name(self), classificationschema_name(other)));
}

static VALUE classificationschema_eq(VALUE self, VALUE other)
{
	return BOOL2RB(rb_obj_is_kind_of(other, cClassificationSchema) && DATA_PTR(self) == DATA_PTR(other));
}

static VALUE classifications_to_a(VALUE self) { return classifications_list(CLASSIFICATIONS(self)); }
static VALUE classifications_each(VALUE self) { su_yield_each(self, classifications_to_a(self)); return Qnil; }
static VALUE classifications_length(VALUE self) { return LONG2NUM(RARRAY_LEN(classifications_to_a(self))); }

static VALUE classifications_keys(VALUE self)
{
	VALUE schemas = classifications_to_a(self);
	VALUE names = rb_ary_new_capa(RARRAY_LEN(schemas));
	for (long i = 0; i < RARRAY_LEN(schemas); i++)
		rb_ary_push(names, classificationschema_name(RARRAY_AREF(schemas, i)));
	return names;
}

static VALUE classifications_aref(VALUE self, VALUE key)
{
	if (!RB_TYPE_P(key, T_STRING))
		return rb_ary_entry(classifications_to_a(self), NUM2LONG(key));
	SUSchemaRef schema = SU_INVALID;
	SUClassificationsGetSchema(CLASSIFICATIONS(self), StringValueCStr(key), &schema);
	return su_wrap_or_nil(cClassificationSchema, schema.ptr);
}

static VALUE classifications_load_schema(VALUE self, VALUE file)
{
	return BOOL2RB(SUClassificationsLoadSchema(CLASSIFICATIONS(self), StringValueCStr(file)) == SU_ERROR_NONE);
}

// Model#classifications
static VALUE model_classifications(VALUE model)
{
	SUClassificationsRef classifications = SU_INVALID;
	SUModelGetClassifications(SU_REF(SUModelRef, model), &classifications);
	return su_wrap_or_nil(cClassifications, classifications.ptr);
}

// ComponentDefinition#add_classification(schema_name, type_name)
static VALUE componentdefinition_add_classification(VALUE self, VALUE schema_name, VALUE type_name)
{
	SUModelRef model = SU_INVALID;
	SUEntityGetModel(SU_REF(SUEntityRef, self), &model);
	SUClassificationsRef classifications = SU_INVALID;
	SUModelGetClassifications(model, &classifications);
	SUSchemaRef schema = SU_INVALID;
	SUSchemaTypeRef type = SU_INVALID;
	if (SUClassificationsGetSchema(classifications, StringValueCStr(schema_name), &schema) != SU_ERROR_NONE ||
		SUSchemaGetSchemaType(schema, StringValueCStr(type_name), &type) != SU_ERROR_NONE)
		return Qfalse;
	return BOOL2RB(SUComponentDefinitionApplySchemaType(SU_REF(SUComponentDefinitionRef, self), schema, type) == SU_ERROR_NONE);
}

void Init_classifications(void)
{
	rb_include_module(cClassificationSchema, rb_mComparable);
	rb_define_method(cClassificationSchema, "name", classificationschema_name, 0);
	rb_define_method(cClassificationSchema, "<=>", classificationschema_cmp, 1);
	rb_define_method(cClassificationSchema, "==", classificationschema_eq, 1);
	rb_include_module(cClassifications, rb_mEnumerable);
	rb_define_method(cClassifications, "each", classifications_each, 0);
	rb_define_method(cClassifications, "[]", classifications_aref, 1);
	rb_define_method(cClassifications, "keys", classifications_keys, 0);
	rb_define_method(cClassifications, "length", classifications_length, 0);
	rb_define_method(cClassifications, "size", classifications_length, 0);
	rb_define_method(cClassifications, "load_schema", classifications_load_schema, 1);
	rb_define_method(cModel, "classifications", model_classifications, 0);
	rb_define_method(cComponentDefinition, "add_classification", componentdefinition_add_classification, 2);
}
