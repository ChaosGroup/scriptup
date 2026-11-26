#include <scriptup.h>

static VALUE model_close(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 0, 1);
	SUModelRef model = Sketchup_get_active_model();
	enum SUResult result = SUModelRelease(&model);
	Sketchup_set_active_model(model);
	return result == SU_ERROR_NONE ? Qnil : Qfalse;
}

static VALUE model_entities(VALUE self)
{
	SUEntitiesRef entities = SU_INVALID;
	SUModelGetEntities(SU_REF(SUModelRef, self), &entities);
	return su_entities_new(entities);
}

static VALUE model_layers(VALUE self)
{
	return su_wrap(cLayers, DATA_PTR(self));
}

static VALUE model_line_styles(VALUE self)
{
	SULineStylesRef styles = SU_INVALID;
	SUModelGetLineStyles(SU_REF(SUModelRef, self), &styles);
	return su_wrap_or_nil(cLineStyles, styles.ptr);
}

static VALUE model_materials(VALUE self)
{
	return su_wrap(cMaterials, DATA_PTR(self));
}

static VALUE model_definitions(VALUE self)
{
	return su_wrap(cDefinitionList, DATA_PTR(self));
}

static VALUE model_attribute_dictionaries(VALUE self)
{
	return su_model_attribute_dictionaries(SU_REF(SUModelRef, self));
}

static VALUE model_eq(VALUE self, VALUE other)
{
	return BOOL2RB(rb_obj_is_kind_of(other, cModel) && DATA_PTR(self) == DATA_PTR(other));
}

static VALUE model_hash(VALUE self)
{
	return rb_funcall(ULL2NUM((unsigned long long)(uintptr_t)DATA_PTR(self)), (rb_intern)("hash"), 0);
}

static VALUE model_active_entities(VALUE self)
{
	SUEntitiesRef entities = SU_INVALID;
	SUModelGetActiveEntities(SU_REF(SUModelRef, self), &entities);
	return su_entities_new(entities);
}

DEFINE_REF_GETTER(model_active_layer, SUModelRef, SUModelGetActiveLayer, SULayerRef, cLayer)
DEFINE_REF_LIST(model_section_list, SUModelRef, SUModelGetNumActiveSectionPlanes, SUModelGetActiveSectionPlanes, SUSectionPlaneRef, cSectionPlane)
DEFINE_STRING_GETTER(model_name, SUModelRef, SUModelGetName)
DEFINE_STRING_SETTER(model_set_name, SUModelRef, SUModelSetName)
DEFINE_STRING_GETTER(model_title, SUModelRef, SUModelGetTitle)
DEFINE_STRING_GETTER(model_path, SUModelRef, SUModelGetPath)
DEFINE_STRING_GETTER(model_guid, SUModelRef, SUModelGetGuid)
DEFINE_STRING_GETTER(model_description, SUModelRef, SUModelGetDescription)
DEFINE_STRING_SETTER(model_set_description, SUModelRef, SUModelSetDescription)
DEFINE_BOOL_GETTER(model_georeferenced_p, SUModelRef, SUModelIsGeoReferenced)

static VALUE model_set_active_layer(VALUE self, VALUE layer)
{
	VALUE target = RB_TYPE_P(layer, T_STRING) ? rb_funcall(rb_funcall(self, (rb_intern)("layers"), 0), (rb_intern)("[]"), 1, layer) : layer;
	if (NIL_P(target))
		rb_raise(rb_eArgError, "layer not found: %"PRIsVALUE, layer);
	SUModelSetActiveLayer(SU_REF(SUModelRef, self), (SULayerRef){su_unwrap(target, cLayer)});
	return layer;
}

static VALUE model_active_section_planes(VALUE self)
{
	return model_section_list(SU_REF(SUModelRef, self));
}

static VALUE model_valid_p(VALUE self)
{
	return BOOL2RB(DATA_PTR(self) != NULL && DATA_PTR(self) == Sketchup_get_active_model().ptr);
}

static VALUE model_bounds(VALUE self)
{
	SUEntitiesRef entities = SU_INVALID;
	SUModelGetEntities(SU_REF(SUModelRef, self), &entities);
	struct SUBoundingBox3D bbox;
	SUEntitiesGetBoundingBox(entities, &bbox);
	return su_bounding_box_new(bbox);
}

static VALUE model_number_faces(VALUE self)
{
	struct SUModelStatistics statistics;
	memset(&statistics, 0, sizeof(statistics));
	SUModelGetStatistics(SU_REF(SUModelRef, self), &statistics);
	return INT2NUM(statistics.entity_counts[SUEntityType_Face]);
}

static VALUE model_attribute_dictionary(int argc, VALUE* argv, VALUE self)
{
	VALUE name, create;
	rb_scan_args(argc, argv, "11", &name, &create);
	VALUE found = su_find_by(rb_funcall(rb_funcall(self, (rb_intern)("attribute_dictionaries"), 0), (rb_intern)("to_a"), 0),
		"name", StringValue(name));
	if (!NIL_P(found) || !RTEST(create))
		return found;
	SUAttributeDictionaryRef dictionary = SU_INVALID;
	SUModelGetAttributeDictionary(SU_REF(SUModelRef, self), StringValueCStr(name), &dictionary);
	return su_wrap_or_nil(cAttributeDictionary, dictionary.ptr);
}

static VALUE model_get_attribute(int argc, VALUE* argv, VALUE self)
{
	VALUE name, key, default_value;
	rb_scan_args(argc, argv, "21", &name, &key, &default_value);
	VALUE dictionary = model_attribute_dictionary(1, &name, self);
	VALUE value = NIL_P(dictionary) ? Qnil : rb_funcall(dictionary, (rb_intern)("[]"), 1, key);
	return NIL_P(value) ? default_value : value;
}

static VALUE model_set_attribute(VALUE self, VALUE name, VALUE key, VALUE value)
{
	VALUE args[2] = {name, Qtrue};
	rb_funcall(model_attribute_dictionary(2, args, self), (rb_intern)("[]="), 2, key, value);
	return value;
}

// One id returns an entity (or nil); several return an array.
static VALUE model_find_entity_by_persistent_id(int argc, VALUE* argv, VALUE self)
{
	VALUE ids = argc == 1 && RB_TYPE_P(argv[0], T_ARRAY) ? argv[0] : rb_ary_new_from_values(argc, argv);
	long count = RARRAY_LEN(ids);
	if (count == 0)
		rb_raise(rb_eArgError, "at least one persistent id is required");
	int64_t* pids = ALLOC_N(int64_t, count);
	SUEntityRef* entities = ALLOC_N(SUEntityRef, count);
	for (long i = 0; i < count; i++)
	{
		pids[i] = NUM2LL(RARRAY_AREF(ids, i));
		entities[i].ptr = NULL;
	}
	SUModelGetEntitiesOfTypeByPersistentIDs(SU_REF(SUModelRef, self), FLAG_GET_ENTITIES_TYPE_ALL, (size_t)count, pids, entities);
	VALUE result = rb_ary_new_capa(count);
	for (long i = 0; i < count; i++)
		rb_ary_push(result, su_wrap_entity(entities[i]));
	xfree(pids);
	xfree(entities);
	return count == 1 && !RB_TYPE_P(argv[0], T_ARRAY) ? RARRAY_AREF(result, 0) : result;
}

static VALUE model_instance_path_from_pid_path(VALUE self, VALUE pid_path)
{
	SUStringRef pid = SU_INVALID;
	SUStringCreateFromUTF8(&pid, StringValueCStr(pid_path));
	SUInstancePathRef path = SU_INVALID;
	SUInstancePathCreate(&path);
	enum SUResult result = SUModelGetInstancePathByPid(SU_REF(SUModelRef, self), pid, &path);
	SUStringRelease(&pid);
	VALUE wrapped = result == SU_ERROR_NONE ? su_instance_path_copy(path) : Qnil;
	SUInstancePathRelease(&path);
	if (NIL_P(wrapped))
		rb_raise(rb_eArgError, "no instance path for %"PRIsVALUE, pid_path);
	return wrapped;
}

static VALUE model_drawing_element_visible_p(VALUE self, VALUE instance_path)
{
	bool visible = false;
	if (SUModelIsDrawingElementVisible(SU_REF(SUModelRef, self), su_to_instance_path(instance_path), &visible) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "invalid instance path");
	return BOOL2RB(visible);
}

static VALUE model_save(int argc, VALUE* argv, VALUE self)
{
	VALUE path, version;
	rb_scan_args(argc, argv, "02", &path, &version);
	if (NIL_P(path))
		path = model_path(self);
	if (RSTRING_LEN(StringValue(path)) == 0)
		rb_raise(rb_eArgError, "the model has no path; pass one");
	enum SUResult result = NIL_P(version)
		? SUModelSaveToFile(SU_REF(SUModelRef, self), StringValueCStr(path))
		: SUModelSaveToFileWithVersion(SU_REF(SUModelRef, self), StringValueCStr(path), (enum SUModelVersion)NUM2INT(version));
	return BOOL2RB(result == SU_ERROR_NONE);
}

static VALUE model_behavior(VALUE self)
{
	VALUE behavior = su_wrap(cBehavior, DATA_PTR(self));
	rb_ivar_set(behavior, (rb_intern)("__owner_is_model"), Qtrue);
	return behavior;
}

void Init_model(void)
{
	static const struct { const char* name; VALUE (*fn)(ANYARGS); int argc; } methods[] = {
		{"active_entities", RUBY_METHOD_FUNC(model_active_entities), 0},
		{"active_layer", RUBY_METHOD_FUNC(model_active_layer), 0},
		{"active_layer=", RUBY_METHOD_FUNC(model_set_active_layer), 1},
		{"active_section_planes", RUBY_METHOD_FUNC(model_active_section_planes), 0},
		{"name", RUBY_METHOD_FUNC(model_name), 0},
		{"name=", RUBY_METHOD_FUNC(model_set_name), 1},
		{"title", RUBY_METHOD_FUNC(model_title), 0},
		{"path", RUBY_METHOD_FUNC(model_path), 0},
		{"guid", RUBY_METHOD_FUNC(model_guid), 0},
		{"description", RUBY_METHOD_FUNC(model_description), 0},
		{"description=", RUBY_METHOD_FUNC(model_set_description), 1},
		{"georeferenced?", RUBY_METHOD_FUNC(model_georeferenced_p), 0},
		{"valid?", RUBY_METHOD_FUNC(model_valid_p), 0},
		{"bounds", RUBY_METHOD_FUNC(model_bounds), 0},
		{"number_faces", RUBY_METHOD_FUNC(model_number_faces), 0},
		{"attribute_dictionary", RUBY_METHOD_FUNC(model_attribute_dictionary), -1},
		{"get_attribute", RUBY_METHOD_FUNC(model_get_attribute), -1},
		{"set_attribute", RUBY_METHOD_FUNC(model_set_attribute), 3},
		{"find_entity_by_persistent_id", RUBY_METHOD_FUNC(model_find_entity_by_persistent_id), -1},
		{"instance_path_from_pid_path", RUBY_METHOD_FUNC(model_instance_path_from_pid_path), 1},
		{"drawing_element_visible?", RUBY_METHOD_FUNC(model_drawing_element_visible_p), 1},
		{"save", RUBY_METHOD_FUNC(model_save), -1},
		{"save_copy", RUBY_METHOD_FUNC(model_save), -1},
		{"behavior", RUBY_METHOD_FUNC(model_behavior), 0},
	};
	for (size_t i = 0; i < sizeof(methods) / sizeof(methods[0]); i++)
		rb_define_method(cModel, methods[i].name, methods[i].fn, methods[i].argc);
	rb_define_method(cModel, "==", model_eq, 1);
	rb_define_method(cModel, "eql?", model_eq, 1);
	rb_define_method(cModel, "hash", model_hash, 0);
	rb_define_const(cModel, "LOAD_STATUS_SUCCESS", INT2FIX(SUModelLoadStatus_Success));
	rb_define_const(cModel, "LOAD_STATUS_SUCCESS_MORE_RECENT", INT2FIX(SUModelLoadStatus_Success_MoreRecent));
	rb_define_method(cModel, "close", model_close, -1);
	rb_define_method(cModel, "entities", model_entities, 0);
	rb_define_method(cModel, "layers", model_layers, 0);
	rb_define_method(cModel, "tags", model_layers, 0);
	rb_define_method(cModel, "line_styles", model_line_styles, 0);
	rb_define_method(cModel, "materials", model_materials, 0);
	rb_define_method(cModel, "definitions", model_definitions, 0);
	rb_define_method(cModel, "attribute_dictionaries", model_attribute_dictionaries, 0);
}
