#include <scriptup.h>

#define REF_TYPE_NAMES(X) \
	X(AttributeDictionary) X(Camera) X(ComponentDefinition) X(ComponentInstance) X(Curve) X(Edge) X(EdgeUse) \
	X(Entities) X(Face) X(Group) X(Image) X(Layer) X(Location) X(Loop) X(MeshHelper) X(Material) X(Model) \
	X(Polyline3D) X(Scene) X(Texture) X(TextureWriter) X(TypedValue) X(UVHelper) X(Vertex) X(RenderingOptions) \
	X(GuidePoint) X(GuideLine) X(Schema) X(SchemaType) X(ShadowInfo) X(Axes) X(ArcCurve) X(SectionPlane) \
	X(DynamicComponentInfo) X(DynamicComponentAttribute) X(Style) X(Styles) X(ImageRep) X(InstancePath) X(Font) \
	X(Dimension) X(DimensionLinear) X(DimensionRadial) X(DimensionStyle) X(Text) X(EntityList) \
	X(EntityListIterator) X(DrawingElement) X(Entity) X(LengthFormatter) X(LineStyle) X(LineStyleManager) \
	X(Selection) X(LayerFolder) X(Environment) X(Environments) X(Snap)

static const char* ref_type_name(enum SURefType type)
{
	switch (type)
	{
#define REF_TYPE_CASE(name) case SURefType_##name: return #name;
		REF_TYPE_NAMES(REF_TYPE_CASE)
		default: return "Unknown";
	}
}

DEFINE_GETTER(entity_persistent_id, SUEntityRef, SUEntityGetPersistentID, int64_t, 0, LL2NUM)
DEFINE_GETTER(entity_entityID, SUEntityRef, SUEntityGetID, int32_t, 0, INT2NUM)
DEFINE_REF_GETTER(entity_model, SUEntityRef, SUEntityGetModel, SUModelRef, cModel)
DEFINE_REF_LIST(entity_dictionary_list, SUEntityRef, SUEntityGetNumAttributeDictionaries,
	SUEntityGetAttributeDictionaries, SUAttributeDictionaryRef, cAttributeDictionary)

static VALUE entity_typename(VALUE self)
{
	return rb_str_new_cstr(ref_type_name(SUEntityGetType(SU_REF(SUEntityRef, self))));
}

static VALUE entity_parent(VALUE self)
{
	SUEntitiesRef entities = SU_INVALID;
	SUEntityGetParentEntities(SU_REF(SUEntityRef, self), &entities);
	return su_wrap(cDefinitionList, entities.ptr);
}

static VALUE entity_observers(VALUE self)
{
	ID id = (rb_intern)("@observers");
	VALUE observers = rb_attr_get(self, id);
	if (NIL_P(observers))
	{
		observers = rb_hash_new();
		rb_ivar_set(self, id, observers);
	}
	return observers;
}

static VALUE entity_add_observer(VALUE self, VALUE observer)
{
	rb_hash_aset(entity_observers(self), observer, observer);
	return Qtrue;
}

static VALUE entity_remove_observer(VALUE self, VALUE observer)
{
	return BOOL2RB(rb_hash_delete(entity_observers(self), observer) != Qnil);
}

static VALUE entity_attribute_dictionaries(VALUE self)
{
	return su_wrap(cAttributeDictionaries, DATA_PTR(self));
}

static VALUE entity_find_dictionary(VALUE self, VALUE name)
{
	return su_find_by(entity_dictionary_list(SU_REF(SUEntityRef, self)), "name", name);
}

static VALUE entity_attribute_dictionary(int argc, VALUE* argv, VALUE self)
{
	VALUE name;
	VALUE create;
	rb_scan_args(argc, argv, "11", &name, &create);
	if (rb_obj_is_kind_of(self, cMaterials) || rb_obj_is_kind_of(self, cDefinitionList))
		return Qnil;

	VALUE dictionary = entity_find_dictionary(self, name);
	if (NIL_P(dictionary) && RTEST(create))
	{
		SUAttributeDictionaryRef created = SU_INVALID;
		SUAttributeDictionaryCreate(&created, StringValueCStr(name));
		SUEntityAddAttributeDictionary(SU_REF(SUEntityRef, self), created);
		dictionary = su_wrap(cAttributeDictionary, created.ptr);
	}
	return dictionary;
}

static VALUE entity_delete_attribute(int argc, VALUE* argv, VALUE self)
{
	VALUE name;
	VALUE key;
	rb_scan_args(argc, argv, "11", &name, &key);
	const char* name_ptr = StringValueCStr(name);
	if (strcmp(name_ptr, "SU_InstanceSet") == 0 || strcmp(name_ptr, "SU_DefinitionSet") == 0)
		rb_raise(rb_eArgError, "Cannot delete internal");
	if (!NIL_P(key))
		return Qfalse; // ToDo: delete a single key

	VALUE dictionary = entity_find_dictionary(self, name);
	if (NIL_P(dictionary))
		return Qfalse;
	SUAttributeDictionaryRef ref = SU_REF(SUAttributeDictionaryRef, dictionary);
	SUAttributeDictionaryRelease(&ref);
	return Qtrue;
}

static VALUE entity_valid_p(VALUE self)
{
	return BOOL2RB(DATA_PTR(self) != NULL);
}

static VALUE entity_deleted_p(VALUE self)
{
	return BOOL2RB(DATA_PTR(self) == NULL);
}

static VALUE entity_get_attribute(int argc, VALUE* argv, VALUE self)
{
	VALUE name;
	VALUE key;
	VALUE default_value;
	rb_scan_args(argc, argv, "21", &name, &key, &default_value);
	return Qnil; // ToDo: use AttributeDictionary#[]
}

static VALUE entity_set_attribute(VALUE self, VALUE name, VALUE key, VALUE value)
{
	return Qnil; // ToDo: use AttributeDictionary#[]=
}

void Init_entity(void)
{
	rb_define_method(cEntity, "persistent_id", entity_persistent_id, 0);
	rb_define_method(cEntity, "entityID", entity_entityID, 0);
	rb_define_method(cEntity, "typename", entity_typename, 0);
	rb_define_method(cEntity, "parent", entity_parent, 0);
	rb_define_method(cEntity, "model", entity_model, 0);
	rb_define_method(cEntity, "add_observer", entity_add_observer, 1);
	rb_define_method(cEntity, "remove_observer", entity_remove_observer, 1);
	rb_define_method(cEntity, "attribute_dictionaries", entity_attribute_dictionaries, 0);
	rb_define_method(cEntity, "attribute_dictionary", entity_attribute_dictionary, -1);
	rb_define_method(cEntity, "delete_attribute", entity_delete_attribute, -1);
	rb_define_method(cEntity, "deleted?", entity_deleted_p, 0);
	rb_define_method(cEntity, "valid?", entity_valid_p, 0);
	rb_define_method(cEntity, "get_attribute", entity_get_attribute, -1);
	rb_define_method(cEntity, "set_attribute", entity_set_attribute, 3);
}
