#include <scriptup.h>

#define SCENE(self) SU_REF(SUSceneRef, self)

DEFINE_STRING_GETTER(page_name, SUSceneRef, SUSceneGetName)
DEFINE_STRING_SETTER(page_set_name, SUSceneRef, SUSceneSetName)
DEFINE_STRING_GETTER(page_description, SUSceneRef, SUSceneGetDescription)
DEFINE_STRING_SETTER(page_set_description, SUSceneRef, SUSceneSetDescription)
DEFINE_REF_GETTER(page_axes, SUSceneRef, SUSceneGetAxes, SUAxesRef, cAxes)
DEFINE_REF_GETTER(page_rendering_options, SUSceneRef, SUSceneGetRenderingOptions, SURenderingOptionsRef, cRenderingOptions)
DEFINE_REF_GETTER(page_shadow_info, SUSceneRef, SUSceneGetShadowInfo, SUShadowInfoRef, cShadowInfo)
DEFINE_REF_GETTER(page_style, SUSceneRef, SUSceneGetStyle, SUStyleRef, cStyle)
DEFINE_REF_GETTER(page_environment, SUSceneRef, SUSceneGetEnvironment, SUEnvironmentRef, cEnvironment)
DEFINE_REF_LIST(page_layer_list, SUSceneRef, SUSceneGetNumLayers, SUSceneGetLayers, SULayerRef, cLayer)
DEFINE_REF_LIST(page_folder_list, SUSceneRef, SUSceneGetNumLayerFolders, SUSceneGetLayerFolders, SULayerFolderRef, cLayerFolder)
DEFINE_REF_LIST(page_section_list, SUSceneRef, SUSceneGetNumActiveSectionPlanes, SUSceneGetActiveSectionPlanes, SUSectionPlaneRef, cSectionPlane)

#define DEFINE_FLAG(name, GetFn, SetFn) \
	DEFINE_BOOL_GETTER(page_##name##_p, SUSceneRef, GetFn) \
	DEFINE_BOOL_SETTER(page_set_##name, SUSceneRef, SetFn)
DEFINE_FLAG(include_in_animation, SUSceneGetIncludeInAnimation, SUSceneSetIncludeInAnimation)
DEFINE_FLAG(use_axes, SUSceneGetUseAxes, SUSceneSetUseAxes)
DEFINE_FLAG(use_camera, SUSceneGetUseCamera, SUSceneSetUseCamera)
DEFINE_FLAG(use_environment, SUSceneGetUseEnvironment, SUSceneSetUseEnvironment)
DEFINE_FLAG(use_hidden, SUSceneGetUseHidden, SUSceneSetUseHidden)
DEFINE_FLAG(use_hidden_geometry, SUSceneGetUseHiddenGeometry, SUSceneSetUseHiddenGeometry)
DEFINE_FLAG(use_hidden_layers, SUSceneGetUseHiddenLayers, SUSceneSetUseHiddenLayers)
DEFINE_FLAG(use_hidden_objects, SUSceneGetUseHiddenObjects, SUSceneSetUseHiddenObjects)
DEFINE_FLAG(use_rendering_options, SUSceneGetUseRenderingOptions, SUSceneSetUseRenderingOptions)
DEFINE_FLAG(use_section_planes, SUSceneGetUseSectionPlanes, SUSceneSetUseSectionPlanes)
DEFINE_FLAG(use_shadow_info, SUSceneGetUseShadowInfo, SUSceneSetUseShadowInfo)

// Page-specific lists return nil when empty, like the Ruby API.
static VALUE nil_if_empty(VALUE ary)
{
	return RARRAY_LEN(ary) == 0 ? Qnil : ary;
}

static VALUE page_layers(VALUE self) { return nil_if_empty(page_layer_list(SCENE(self))); }
static VALUE page_layer_folders(VALUE self) { return nil_if_empty(page_folder_list(SCENE(self))); }
static VALUE page_active_section_planes(VALUE self) { return nil_if_empty(page_section_list(SCENE(self))); }

static VALUE page_hidden_entities(VALUE self)
{
	size_t count = 0;
	SUSceneGetNumHiddenEntities(SCENE(self), &count);
	if (count == 0)
		return Qnil;
	SUEntityRef* entities = ZALLOC_N(SUEntityRef, count);
	SUSceneGetHiddenEntities(SCENE(self), count, entities, &count);
	VALUE ary = rb_ary_new_capa((long)count);
	for (size_t i = 0; i < count; i++)
		rb_ary_push(ary, su_wrap_entity(entities[i]));
	xfree(entities);
	return ary;
}

static VALUE page_camera(VALUE self)
{
	SUCameraRef camera = SU_INVALID;
	SUSceneGetCamera(SCENE(self), &camera);
	return su_camera_wrap(camera);
}

static VALUE page_set_environment(VALUE self, VALUE environment)
{
	SUSceneSetEnvironment(SCENE(self), (SUEnvironmentRef){su_unwrap(environment, cEnvironment)});
	return environment;
}

static VALUE page_get_drawingelement_visibility(VALUE self, VALUE element)
{
	bool hidden = false;
	SUSceneGetDrawingElementHidden(SCENE(self), (SUDrawingElementRef){su_unwrap(element, cDrawingelement)}, &hidden);
	return BOOL2RB(!hidden);
}

static VALUE page_set_drawingelement_visibility(VALUE self, VALUE element, VALUE visible)
{
	enum SUResult result = SUSceneSetDrawingElementHidden(SCENE(self),
		(SUDrawingElementRef){su_unwrap(element, cDrawingelement)}, !RTEST(visible));
	return BOOL2RB(result == SU_ERROR_NONE);
}

// A page lists the layers and folders whose visibility differs; hiding adds, showing removes.
static VALUE page_set_visibility(VALUE self, VALUE target, VALUE visible)
{
	if (rb_obj_is_kind_of(target, cLayerFolder))
	{
		SULayerFolderRef folder = SU_REF(SULayerFolderRef, target);
		RTEST(visible) ? SUSceneRemoveLayerFolder(SCENE(self), folder) : SUSceneAddLayerFolder(SCENE(self), folder);
	}
	else
	{
		SULayerRef layer = {su_unwrap(target, cLayer)};
		RTEST(visible) ? SUSceneRemoveLayer(SCENE(self), layer) : SUSceneAddLayer(SCENE(self), layer);
	}
	return self;
}

static VALUE page_update(int argc, VALUE* argv, VALUE self)
{
	VALUE flags;
	rb_scan_args(argc, argv, "01", &flags);
	unsigned int f = NIL_P(flags) ? FLAG_USE_ALL : NUM2UINT(flags);
	return BOOL2RB(SUSceneUpdate(SCENE(self), f) == SU_ERROR_NONE);
}

void Init_page(void)
{
	static const struct { const char* name; int value; } constants[] = {
		{"PAGE_USE_CAMERA", FLAG_USE_CAMERA}, {"PAGE_USE_RENDERING_OPTIONS", FLAG_USE_RENDERING_OPTIONS},
		{"PAGE_USE_SHADOWINFO", FLAG_USE_SHADOWINFO}, {"PAGE_USE_SKETCHCS", FLAG_USE_AXES},
		{"PAGE_USE_HIDDEN", FLAG_USE_HIDDEN}, {"PAGE_USE_LAYER_VISIBILITY", FLAG_USE_LAYER_VISIBILITY},
		{"PAGE_USE_SECTION_PLANES", FLAG_USE_SECTION_PLANES}, {"PAGE_USE_HIDDEN_GEOMETRY", FLAG_USE_HIDDEN_GEOMETRY},
		{"PAGE_USE_HIDDEN_OBJECTS", FLAG_USE_HIDDEN_OBJECTS}, {"PAGE_USE_ENVIRONMENT", FLAG_USE_ENVIRONMENT},
		{"PAGE_USE_ALL", FLAG_USE_ALL}, {"PAGE_NO_CAMERA", FLAG_USE_ALL & ~FLAG_USE_CAMERA},
	};
	for (size_t i = 0; i < sizeof(constants) / sizeof(constants[0]); i++)
		rb_define_const(rb_cObject, constants[i].name, INT2FIX(constants[i].value));

	static const struct { const char* name; VALUE (*fn)(ANYARGS); int argc; } methods[] = {
		{"name", RUBY_METHOD_FUNC(page_name), 0},
		{"name=", RUBY_METHOD_FUNC(page_set_name), 1},
		{"label", RUBY_METHOD_FUNC(page_name), 0},
		{"description", RUBY_METHOD_FUNC(page_description), 0},
		{"description=", RUBY_METHOD_FUNC(page_set_description), 1},
		{"axes", RUBY_METHOD_FUNC(page_axes), 0},
		{"camera", RUBY_METHOD_FUNC(page_camera), 0},
		{"rendering_options", RUBY_METHOD_FUNC(page_rendering_options), 0},
		{"shadow_info", RUBY_METHOD_FUNC(page_shadow_info), 0},
		{"style", RUBY_METHOD_FUNC(page_style), 0},
		{"environment", RUBY_METHOD_FUNC(page_environment), 0},
		{"environment=", RUBY_METHOD_FUNC(page_set_environment), 1},
		{"layers", RUBY_METHOD_FUNC(page_layers), 0},
		{"layer_folders", RUBY_METHOD_FUNC(page_layer_folders), 0},
		{"active_section_planes", RUBY_METHOD_FUNC(page_active_section_planes), 0},
		{"hidden_entities", RUBY_METHOD_FUNC(page_hidden_entities), 0},
		{"get_drawingelement_visibility", RUBY_METHOD_FUNC(page_get_drawingelement_visibility), 1},
		{"set_drawingelement_visibility", RUBY_METHOD_FUNC(page_set_drawingelement_visibility), 2},
		{"set_visibility", RUBY_METHOD_FUNC(page_set_visibility), 2},
		{"update", RUBY_METHOD_FUNC(page_update), -1},
		{"include_in_animation?", RUBY_METHOD_FUNC(page_include_in_animation_p), 0},
		{"include_in_animation=", RUBY_METHOD_FUNC(page_set_include_in_animation), 1},
		{"use_axes?", RUBY_METHOD_FUNC(page_use_axes_p), 0},
		{"use_axes=", RUBY_METHOD_FUNC(page_set_use_axes), 1},
		{"use_camera?", RUBY_METHOD_FUNC(page_use_camera_p), 0},
		{"use_camera=", RUBY_METHOD_FUNC(page_set_use_camera), 1},
		{"use_environment?", RUBY_METHOD_FUNC(page_use_environment_p), 0},
		{"use_environment=", RUBY_METHOD_FUNC(page_set_use_environment), 1},
		{"use_hidden?", RUBY_METHOD_FUNC(page_use_hidden_p), 0},
		{"use_hidden=", RUBY_METHOD_FUNC(page_set_use_hidden), 1},
		{"use_hidden_geometry?", RUBY_METHOD_FUNC(page_use_hidden_geometry_p), 0},
		{"use_hidden_geometry=", RUBY_METHOD_FUNC(page_set_use_hidden_geometry), 1},
		{"use_hidden_layers?", RUBY_METHOD_FUNC(page_use_hidden_layers_p), 0},
		{"use_hidden_layers=", RUBY_METHOD_FUNC(page_set_use_hidden_layers), 1},
		{"use_hidden_objects?", RUBY_METHOD_FUNC(page_use_hidden_objects_p), 0},
		{"use_hidden_objects=", RUBY_METHOD_FUNC(page_set_use_hidden_objects), 1},
		{"use_rendering_options?", RUBY_METHOD_FUNC(page_use_rendering_options_p), 0},
		{"use_rendering_options=", RUBY_METHOD_FUNC(page_set_use_rendering_options), 1},
		{"use_section_planes?", RUBY_METHOD_FUNC(page_use_section_planes_p), 0},
		{"use_section_planes=", RUBY_METHOD_FUNC(page_set_use_section_planes), 1},
		{"use_shadow_info?", RUBY_METHOD_FUNC(page_use_shadow_info_p), 0},
		{"use_shadow_info=", RUBY_METHOD_FUNC(page_set_use_shadow_info), 1},
	};
	for (size_t i = 0; i < sizeof(methods) / sizeof(methods[0]); i++)
		rb_define_method(cPage, methods[i].name, methods[i].fn, methods[i].argc);
}
