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

static const struct { enum SURefType type; VALUE* klass; } entity_classes[] = {
	{SURefType_ArcCurve, &cArcCurve},
	{SURefType_AttributeDictionary, &cAttributeDictionary},
	{SURefType_Axes, &cAxes},
	{SURefType_Curve, &cCurve},
	{SURefType_EdgeUse, &cEdgeUse},
	{SURefType_Loop, &cLoop},
	{SURefType_Vertex, &cVertex},
	{SURefType_ComponentDefinition, &cComponentDefinition},
	{SURefType_ComponentInstance, &cComponentInstance},
	{SURefType_Dimension, &cDimension},
	{SURefType_DimensionLinear, &cDimension},
	{SURefType_DimensionRadial, &cDimension},
	{SURefType_Edge, &cEdge},
	{SURefType_Face, &cFace},
	{SURefType_Group, &cGroup},
	{SURefType_GuideLine, &cConstructionLine},
	{SURefType_GuidePoint, &cConstructionPoint},
	{SURefType_Image, &cImage},
	{SURefType_Layer, &cLayer},
	{SURefType_LayerFolder, &cLayerFolder},
	{SURefType_LineStyle, &cLineStyle},
	{SURefType_Material, &cMaterial},
	{SURefType_SectionPlane, &cSectionPlane},
	{SURefType_Text, &cText},
	{SURefType_Texture, &cTexture},
};

// Wraps an entity as the Ruby class matching its runtime type.
VALUE su_wrap_entity(SUEntityRef entity)
{
	if (SUIsInvalid(entity))
		return Qnil;
	enum SURefType type = SUEntityGetType(entity);
	for (size_t i = 0; i < sizeof(entity_classes) / sizeof(entity_classes[0]); i++)
		if (entity_classes[i].type == type)
			return su_wrap(*entity_classes[i].klass, entity.ptr);
	return su_wrap(cEntity, entity.ptr);
}

static VALUE entity_parent(VALUE self)
{
	SUEntityRef entity = SU_REF(SUEntityRef, self);
	if (SUEntityGetType(entity) == SURefType_ComponentDefinition)
	{
		SUModelRef model = SU_INVALID;
		SUEntityGetModel(entity, &model);
		return su_wrap_or_nil(cDefinitionList, model.ptr);
	}
	SUEntitiesRef entities = SU_INVALID;
	struct SUEntitiesParent parent = {SU_INVALID, SU_INVALID};
	if (SUEntityGetParentEntities(entity, &entities) != SU_ERROR_NONE || SUEntitiesGetParent(entities, &parent) != SU_ERROR_NONE)
		return Qnil;
	if (SUIsValid(parent.definition))
		return su_wrap(cComponentDefinition, parent.definition.ptr);
	return su_wrap_or_nil(cModel, parent.model.ptr);
}

static VALUE entity_eq(VALUE self, VALUE other)
{
	return BOOL2RB(rb_obj_is_kind_of(other, cEntity) && DATA_PTR(self) == DATA_PTR(other));
}

static VALUE entity_hash(VALUE self)
{
	return rb_funcall(ULL2NUM((unsigned long long)(uintptr_t)DATA_PTR(self)), (rb_intern)("hash"), 0);
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
	StringValue(name);
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
		rb_raise(rb_eNotImpError, "the C API cannot delete a single attribute key");

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
	StringValue(key);
	VALUE dictionary = entity_find_dictionary(self, name);
	VALUE value = NIL_P(dictionary) ? Qnil : rb_funcall(dictionary, (rb_intern)("[]"), 1, key);
	return NIL_P(value) ? default_value : value;
}

static VALUE entity_set_attribute(VALUE self, VALUE name, VALUE key, VALUE value)
{
	VALUE args[2] = {name, Qtrue};
	VALUE dictionary = entity_attribute_dictionary(2, args, self);
	if (NIL_P(dictionary))
		rb_raise(rb_eArgError, "cannot set attributes on %"PRIsVALUE, rb_obj_class(self));
	rb_funcall(dictionary, (rb_intern)("[]="), 2, key, value);
	return value;
}

void Init_entity(void)
{
	rb_define_method(cEntity, "persistent_id", entity_persistent_id, 0);
	rb_define_method(cEntity, "entityID", entity_entityID, 0);
	rb_define_method(cEntity, "typename", entity_typename, 0);
	rb_define_method(cEntity, "parent", entity_parent, 0);
	rb_define_method(cEntity, "model", entity_model, 0);
	rb_define_method(cEntity, "==", entity_eq, 1);
	rb_define_method(cEntity, "eql?", entity_eq, 1);
	rb_define_method(cEntity, "hash", entity_hash, 0);
	rb_define_method(cEntity, "attribute_dictionaries", entity_attribute_dictionaries, 0);
	rb_define_method(cEntity, "attribute_dictionary", entity_attribute_dictionary, -1);
	rb_define_method(cEntity, "delete_attribute", entity_delete_attribute, -1);
	rb_define_method(cEntity, "deleted?", entity_deleted_p, 0);
	rb_define_method(cEntity, "valid?", entity_valid_p, 0);
	rb_define_method(cEntity, "get_attribute", entity_get_attribute, -1);
	rb_define_method(cEntity, "set_attribute", entity_set_attribute, 3);
}
