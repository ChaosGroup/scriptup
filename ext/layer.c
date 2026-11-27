#include <scriptup.h>

#define LAYER(self) SU_REF(SULayerRef, self)

DEFINE_STRING_GETTER(layer_name, SULayerRef, SULayerGetName)
DEFINE_STRING_SETTER(layer_set_name, SULayerRef, SULayerSetName)
DEFINE_STRING_GETTER(layer_display_name, SULayerRef, SULayerGetDisplayName)
DEFINE_BOOL_GETTER(layer_visible_p, SULayerRef, SULayerGetVisibility)
DEFINE_BOOL_SETTER(layer_set_visible, SULayerRef, SULayerSetVisibility)
DEFINE_REF_GETTER(layer_folder, SULayerRef, SULayerGetParentLayerFolder, SULayerFolderRef, cLayerFolder)
DEFINE_REF_GETTER(layer_line_style, SULayerRef, SULayerGetLineStyle, SULineStyleRef, cLineStyle)

static VALUE layer_cmp(VALUE self, VALUE other)
{
	if (!rb_obj_is_kind_of(other, cLayer))
		return Qnil;
	return INT2FIX(rb_str_cmp(layer_name(self), layer_name(other)));
}

static VALUE layer_color(VALUE self)
{
	SUMaterialRef material = SU_INVALID;
	SUColor color = {0, 0, 0, 255};
	SULayerGetMaterial(LAYER(self), &material);
	SUMaterialGetColor(material, &color);
	return su_color_new(color);
}

static VALUE layer_set_color(VALUE self, VALUE color)
{
	SUMaterialRef material = SU_INVALID;
	SULayerGetMaterial(LAYER(self), &material);
	SUMaterialSetColor(material, DATA_PTR(su_color_coerce(color)));
	return color;
}

static VALUE layer_set_line_style(VALUE self, VALUE style)
{
	if (NIL_P(style))
		SULayerClearLineStyle(LAYER(self));
	else
		SULayerSetLineStyle(LAYER(self), (SULineStyleRef){su_unwrap(style, cLineStyle)});
	return style;
}

static VALUE layer_set_folder(VALUE self, VALUE folder)
{
	SULayerFolderRef current = SU_INVALID;
	SULayerGetParentLayerFolder(LAYER(self), &current);
	SULayerFolderRef target = {NIL_P(folder) ? NULL : su_unwrap(folder, cLayerFolder)};
	if (SUIsValid(current))
		SULayerFolderRemoveLayer(current, LAYER(self));
	if (SUIsValid(target))
		SULayerFolderAddLayer(target, LAYER(self));
	return folder;
}

static VALUE layer_page_behavior(VALUE self)
{
	enum SULayerVisibilityDefaultType default_type = SULayerVisibilityDefaultType_Visible;
	enum SULayerVisibilityNewSceneType new_scene_type = SULayerVisibilityNewSceneType_LayerDefault;
	SULayerGetSceneBehavior(LAYER(self), &default_type, &new_scene_type);
	return INT2FIX((int)default_type | (int)new_scene_type);
}

static VALUE layer_set_page_behavior(VALUE self, VALUE behavior)
{
	int flags = NUM2INT(behavior);
	enum SULayerVisibilityDefaultType default_type = (flags & 0x1) ? SULayerVisibilityDefaultType_Hidden : SULayerVisibilityDefaultType_Visible;
	enum SULayerVisibilityNewSceneType new_scene_type = (flags & 0x20) ? SULayerVisibilityNewSceneType_Hidden
		: (flags & 0x10) ? SULayerVisibilityNewSceneType_Visible : SULayerVisibilityNewSceneType_LayerDefault;
	SULayerSetSceneBehavior(LAYER(self), default_type, new_scene_type);
	return behavior;
}

void Init_layer(void)
{
	rb_define_const(rb_cObject, "LAYER_VISIBLE_BY_DEFAULT", INT2FIX(SULayerVisibilityDefaultType_Visible));
	rb_define_const(rb_cObject, "LAYER_HIDDEN_BY_DEFAULT", INT2FIX(SULayerVisibilityDefaultType_Hidden));
	rb_define_const(rb_cObject, "LAYER_USES_DEFAULT_VISIBILITY_ON_NEW_PAGES", INT2FIX(SULayerVisibilityNewSceneType_LayerDefault));
	rb_define_const(rb_cObject, "LAYER_IS_VISIBLE_ON_NEW_PAGES", INT2FIX(SULayerVisibilityNewSceneType_Visible));
	rb_define_const(rb_cObject, "LAYER_IS_HIDDEN_ON_NEW_PAGES", INT2FIX(SULayerVisibilityNewSceneType_Hidden));
	rb_include_module(cLayer, rb_mComparable);
	rb_define_method(cLayer, "name", layer_name, 0);
	rb_define_method(cLayer, "name=", layer_set_name, 1);
	rb_define_method(cLayer, "display_name", layer_display_name, 0);
	rb_define_method(cLayer, "<=>", layer_cmp, 1);
	rb_define_method(cLayer, "visible?", layer_visible_p, 0);
	rb_define_method(cLayer, "visible=", layer_set_visible, 1);
	rb_define_method(cLayer, "color", layer_color, 0);
	rb_define_method(cLayer, "color=", layer_set_color, 1);
	rb_define_method(cLayer, "line_style", layer_line_style, 0);
	rb_define_method(cLayer, "line_style=", layer_set_line_style, 1);
	rb_define_method(cLayer, "folder", layer_folder, 0);
	rb_define_method(cLayer, "folder=", layer_set_folder, 1);
	rb_define_method(cLayer, "page_behavior", layer_page_behavior, 0);
	rb_define_method(cLayer, "page_behavior=", layer_set_page_behavior, 1);
}
