#include <stdbool.h>
#include <entity.h>
#include <SketchUpAPI/sketchup.h>
#include <utils.h>

static VALUE Sketchup_Entity_persistentId(VALUE self)
{
	SUEntityRef entity = {DATA_PTR(self)};
	int64_t presistent_id = 0;
	SUEntityGetPersistentID(entity, &presistent_id);
	return LONG2NUM(presistent_id);
}

static VALUE Sketchup_Entity_entityId(VALUE self)
{
	SUEntityRef entity = {DATA_PTR(self)};
	int32_t entityId = 0;
	SUEntityGetID(entity, &entityId);
	return LONG2NUM(entityId);
}

static VALUE Sketchup_Entity_typename(VALUE self)
{
	SUEntityRef entity = {DATA_PTR(self)};
	switch (SUEntityGetType(entity))
	{
		case SURefType_Unknown: return rb_str_new2("Unknown");
  		case SURefType_AttributeDictionary: return rb_str_new2("AttributeDictionary");
  		case SURefType_Camera: return rb_str_new2("Camera");
  		case SURefType_ComponentDefinition: return rb_str_new2("ComponentDefinition");
  		case SURefType_ComponentInstance: return rb_str_new2("ComponentInstance");
  		case SURefType_Curve: return rb_str_new2("Curve");
  		case SURefType_Edge: return rb_str_new2("Edge");
  		case SURefType_EdgeUse: return rb_str_new2("EdgeUse");
  		case SURefType_Entities: return rb_str_new2("Entities");
  		case SURefType_Face: return rb_str_new2("Face");
  		case SURefType_Group: return rb_str_new2("Group");
  		case SURefType_Image: return rb_str_new2("Image");
  		case SURefType_Layer: return rb_str_new2("Layer");
  		case SURefType_Location: return rb_str_new2("Location");
  		case SURefType_Loop: return rb_str_new2("Loop");
  		case SURefType_MeshHelper: return rb_str_new2("MeshHelper");
  		case SURefType_Material: return rb_str_new2("Material");
  		case SURefType_Model: return rb_str_new2("Model");
  		case SURefType_Polyline3D: return rb_str_new2("Polyline3D");
  		case SURefType_Scene: return rb_str_new2("Scene");
  		case SURefType_Texture: return rb_str_new2("Texture");
  		case SURefType_TextureWriter: return rb_str_new2("TextureWriter");
  		case SURefType_TypedValue: return rb_str_new2("TypedValue");
  		case SURefType_UVHelper: return rb_str_new2("UVHelper");
  		case SURefType_Vertex: return rb_str_new2("Vertex");
  		case SURefType_RenderingOptions: return rb_str_new2("RenderingOptions");
  		case SURefType_GuidePoint: return rb_str_new2("GuidePoint");
  		case SURefType_GuideLine: return rb_str_new2("GuideLine");
  		case SURefType_Schema: return rb_str_new2("Schema");
  		case SURefType_SchemaType: return rb_str_new2("SchemaType");
  		case SURefType_ShadowInfo: return rb_str_new2("ShadowInfo");
  		case SURefType_Axes: return rb_str_new2("Axes");
  		case SURefType_ArcCurve: return rb_str_new2("ArcCurve");
  		case SURefType_SectionPlane: return rb_str_new2("SectionPlane");
  		case SURefType_DynamicComponentInfo: return rb_str_new2("DynamicComponentInfo");
  		case SURefType_DynamicComponentAttribute: return rb_str_new2("DynamicComponentAttribute");
  		case SURefType_Style: return rb_str_new2("Style");
  		case SURefType_Styles: return rb_str_new2("Styles");
  		case SURefType_ImageRep: return rb_str_new2("ImageRep");
  		case SURefType_InstancePath: return rb_str_new2("InstancePath");
  		case SURefType_Font: return rb_str_new2("Font");
  		case SURefType_Dimension: return rb_str_new2("Dimension");
  		case SURefType_DimensionLinear: return rb_str_new2("DimensionLinear");
  		case SURefType_DimensionRadial: return rb_str_new2("DimensionRadial");
  		case SURefType_DimensionStyle: return rb_str_new2("DimensionStyle");
  		case SURefType_Text: return rb_str_new2("Text");
  		case SURefType_EntityList: return rb_str_new2("EntityList");
  		case SURefType_EntityListIterator: return rb_str_new2("EntityListIterator");
  		case SURefType_DrawingElement: return rb_str_new2("DrawingElement");
  		case SURefType_Entity: return rb_str_new2("Entity");
  		case SURefType_LengthFormatter: return rb_str_new2("LengthFormatter");
  		case SURefType_LineStyle: return rb_str_new2("LineStyle");
  		case SURefType_LineStyleManager: return rb_str_new2("LineStyleManager");
  		case SURefType_Selection: return rb_str_new2("Selection");
  		case SURefType_LayerFolder: return rb_str_new2("LayerFolder");
	}
}

static VALUE Sketchup_Entity_parent(VALUE self)
{
	SUEntityRef entity = {DATA_PTR(self)};
	SUEntitiesRef entities = SU_INVALID;
	SUEntityGetParentEntities(entity, &entities);
	return Data_Wrap_Struct(rb_path2class(SKETCHUP_DEFINITIONLIST), 0, 0, entities.ptr);
}

static VALUE Sketchup_Entity_model(VALUE self)
{
	SUEntityRef entity = {DATA_PTR(self)};
	SUModelRef model = SU_INVALID;
	SUEntityGetModel(entity, &model);
	return Data_Wrap_Struct(rb_path2class(SKETCHUP_MODEL), 0, 0, model.ptr);
}

static VALUE Sketchup_Entity_remove_observer(VALUE self, VALUE observer)
{
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wcompound-token-split-by-macro"
	if (!RTEST(rb_ivar_defined(self, rb_intern("@observers"))))
	#pragma clang diagnostic pop
		return Qfalse;

	VALUE result = rb_hash_delete(rb_iv_get(self, "@observers"), observer);

	return RTEST(result) ? Qtrue : Qfalse;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcompound-token-split-by-macro"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
static VALUE Sketchup_Entity_add_observer(VALUE self, VALUE observer)
{
	VALUE observers = Qnil;
	if (RTEST(rb_ivar_defined(self, rb_intern("@observers"))))
	{
		observers = rb_iv_get(self, "@observers");
	}
	else
	{
		observers = rb_hash_new();
		rb_iv_set(self, "@observers", rb_hash_new());
	}
	rb_hash_aset(self, observer, observer);
	return Qtrue;
}
#pragma clang diagnostic pop

void Sketchup_Entity_attribute_dictionaries_Iterator(SUAttributeDictionaryRef dictionary, VALUE* ary)
{
	rb_ary_push(*ary, Data_Wrap_Struct(rb_path2class(SKETCHUP_ATTRIBUTEDICTIONARY), 0, 0, dictionary.ptr));
}

static VALUE Sketchup_Entity_attribute_dictionaries(VALUE self)
{
	SUEntityRef entity = {DATA_PTR(self)};
	return Data_Wrap_Struct(rb_path2class(SKETCHUP_ATTRIBUTEDICTIONARIES), 0, 0, entity.ptr);
}

struct AttributeDictionaryIterationContext
{
	VALUE key;
	SUAttributeDictionaryRef dictionary;
};


void Sketchup_Entity_attribute_dictionary_Iterator(SUAttributeDictionaryRef dictionary, struct AttributeDictionaryIterationContext* attribute_dictionary_struct)
{
	VALUE output;
	GETSTRING(SUAttributeDictionaryGetName, dictionary, output);
	if (strcmp(StringValuePtr(output), StringValuePtr(attribute_dictionary_struct->key)) == 0)
		attribute_dictionary_struct->dictionary = dictionary;
}

static VALUE Sketchup_Entity_attribute_dictionary(int argc, VALUE* argv, VALUE self)
{
	SUEntityRef entity = {DATA_PTR(self)};
	struct AttributeDictionaryIterationContext attribute_dictionary_struct = {argv[0], SU_INVALID};
	FOREACH(SUEntityGetNumAttributeDictionaries, SUEntityGetAttributeDictionaries, SUAttributeDictionaryRef, entity, Sketchup_Entity_attribute_dictionary_Iterator, &attribute_dictionary_struct);
	if (SUIsInvalid(attribute_dictionary_struct.dictionary) && argc > 1 && argv[1])
	{
		SUAttributeDictionaryCreate(&attribute_dictionary_struct.dictionary, StringValuePtr(argv[0]));
		SUEntityAddAttributeDictionary(entity, attribute_dictionary_struct.dictionary);
		return Data_Wrap_Struct(rb_path2class(SKETCHUP_ATTRIBUTEDICTIONARY), 0, 0, attribute_dictionary_struct.dictionary.ptr);
	}
	else if(SUIsValid(attribute_dictionary_struct.dictionary))
		return Data_Wrap_Struct(rb_path2class(SKETCHUP_ATTRIBUTEDICTIONARY), 0, 0, attribute_dictionary_struct.dictionary.ptr);
	else
		return Qnil;
}

static VALUE Sketchup_Entity_delete_attribute(int argc, VALUE* argv, VALUE self)
{
	SUEntityRef entity = {DATA_PTR(self)};
	if (argc == 0)
	{
		rb_raise(rb_eArgError, "Dictionary name required");
	}
	else if (argc == 1 && rb_type(argv[0]) == T_STRING)
	{
		if (strcmp(StringValuePtr(argv[0]), "SU_InstanceSet") == 0 || strcmp(StringValuePtr(argv[0]), "SU_DefinitionSet") == 0)
			rb_raise(rb_eArgError, "Cannot delete internal");

		struct AttributeDictionaryIterationContext attribute_dictionary_struct = {argv[0], SU_INVALID};
		FOREACH(SUEntityGetNumAttributeDictionaries, SUEntityGetAttributeDictionaries, SUAttributeDictionaryRef, entity, Sketchup_Entity_attribute_dictionary_Iterator, &attribute_dictionary_struct);
		if (SUIsValid(attribute_dictionary_struct.dictionary))
		{
			SUAttributeDictionaryRelease(&attribute_dictionary_struct.dictionary);
			return Qtrue;
		}
	}
	else if (argc > 1)
	{
		//ToDo use Sketchup_AttributeDictionary_delete_key
	}
	return Qfalse;
}

static VALUE Sketchup_Entity_deleted(VALUE self)
{
	SUEntityRef entity = {DATA_PTR(self)};
	return SUIsInvalid(entity) ? Qtrue : Qfalse;	
}

static VALUE Sketchup_Entity_valid(VALUE self)
{
	SUEntityRef entity = {DATA_PTR(self)};
	return SUIsValid(entity) ? Qtrue : Qfalse;	
}

static VALUE Sketchup_Entity_get_attribute(int argc, VALUE* argv, VALUE self)
{
	SUEntityRef entity = {DATA_PTR(self)};
	struct AttributeDictionaryIterationContext attribute_dictionary_struct = {argv[0], SU_INVALID};
	FOREACH(SUEntityGetNumAttributeDictionaries, SUEntityGetAttributeDictionaries, SUAttributeDictionaryRef, entity, Sketchup_Entity_attribute_dictionary_Iterator, &attribute_dictionary_struct);
	// use Sketchup_AttributeDictionary_get
	return Qnil;
	
}

static VALUE Sketchup_Entity_set_attribute(int argc, VALUE* argv, VALUE self)
{
	SUEntityRef entity = {DATA_PTR(self)};
	struct AttributeDictionaryIterationContext attribute_dictionary_struct = {argv[0], SU_INVALID};
	FOREACH(SUEntityGetNumAttributeDictionaries, SUEntityGetAttributeDictionaries, SUAttributeDictionaryRef, entity, Sketchup_Entity_attribute_dictionary_Iterator, &attribute_dictionary_struct);
	// use Sketchup_AttributeDictionary_set
	return Qnil;
	
}

VALUE Entity_Init(VALUE Sketchup, VALUE rb_cObject)
{
	VALUE Sketchup_Entity = rb_define_class_under(Sketchup, ENTITY, rb_cObject);
	rb_undef_alloc_func(Sketchup_Entity);
	rb_define_method(Sketchup_Entity, "persistent_id", Sketchup_Entity_persistentId, 0);
	rb_define_method(Sketchup_Entity, "entityID", Sketchup_Entity_entityId, 0);
	rb_define_method(Sketchup_Entity, "typename", Sketchup_Entity_typename, 0);
	rb_define_method(Sketchup_Entity, "parent", Sketchup_Entity_parent, 0);
	rb_define_method(Sketchup_Entity, "model", Sketchup_Entity_model, 0);
	rb_define_method(Sketchup_Entity, "remove_observer", Sketchup_Entity_remove_observer, 1);
	rb_define_method(Sketchup_Entity, "add_observer", Sketchup_Entity_add_observer, 1);
	rb_define_method(Sketchup_Entity, "attribute_dictionaries", Sketchup_Entity_attribute_dictionaries, 0);
	rb_define_method(Sketchup_Entity, "attribute_dictionary", Sketchup_Entity_attribute_dictionary, -1);
	rb_define_method(Sketchup_Entity, "delete_attribute", Sketchup_Entity_delete_attribute, -1);
	rb_define_method(Sketchup_Entity, "deleted?", Sketchup_Entity_deleted, 0);
	rb_define_method(Sketchup_Entity, "valid?", Sketchup_Entity_valid, 0);
	rb_define_method(Sketchup_Entity, "get_attribute", Sketchup_Entity_get_attribute, -1);
	rb_define_method(Sketchup_Entity, "set_attribute", Sketchup_Entity_set_attribute, -1);
	return Sketchup_Entity;
}