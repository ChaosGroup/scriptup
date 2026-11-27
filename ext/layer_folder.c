#include <scriptup.h>

#define FOLDER(self) SU_REF(SULayerFolderRef, self)

DEFINE_STRING_GETTER(layerfolder_name, SULayerFolderRef, SULayerFolderGetName)
DEFINE_STRING_SETTER(layerfolder_set_name, SULayerFolderRef, SULayerFolderSetName)
DEFINE_BOOL_GETTER(layerfolder_visible_p, SULayerFolderRef, SULayerFolderGetVisibility)
DEFINE_BOOL_SETTER(layerfolder_set_visible, SULayerFolderRef, SULayerFolderSetVisibility)
DEFINE_REF_GETTER(layerfolder_folder, SULayerFolderRef, SULayerFolderGetParentLayerFolder, SULayerFolderRef, cLayerFolder)
DEFINE_REF_LIST(layerfolder_layer_list, SULayerFolderRef, SULayerFolderGetNumLayers, SULayerFolderGetLayers, SULayerRef, cLayer)
DEFINE_REF_LIST(layerfolder_folder_list, SULayerFolderRef, SULayerFolderGetNumLayerFolders, SULayerFolderGetLayerFolders, SULayerFolderRef, cLayerFolder)

static VALUE layerfolder_layers(VALUE self) { return layerfolder_layer_list(FOLDER(self)); }
static VALUE layerfolder_folders(VALUE self) { return layerfolder_folder_list(FOLDER(self)); }
static VALUE layerfolder_each_layer(VALUE self) { return su_yield_each(self, layerfolder_layers(self)); }
static VALUE layerfolder_each_folder(VALUE self) { return su_yield_each(self, layerfolder_folders(self)); }
static VALUE layerfolder_count_layers(VALUE self) { return LONG2NUM(RARRAY_LEN(layerfolder_layers(self))); }
static VALUE layerfolder_count_folders(VALUE self) { return LONG2NUM(RARRAY_LEN(layerfolder_folders(self))); }

static VALUE layerfolder_cmp(VALUE self, VALUE other)
{
	if (!rb_obj_is_kind_of(other, cLayerFolder))
		return Qnil;
	return INT2FIX(rb_str_cmp(layerfolder_name(self), layerfolder_name(other)));
}

static VALUE layerfolder_add_layer(VALUE self, VALUE layer)
{
	SULayerRef ref = {su_unwrap(layer, cLayer)};
	SULayerFolderRef current = SU_INVALID;
	SULayerGetParentLayerFolder(ref, &current);
	if (SUIsValid(current))
		SULayerFolderRemoveLayer(current, ref);
	SULayerFolderAddLayer(FOLDER(self), ref);
	return Qnil;
}

static VALUE layerfolder_remove_layer(VALUE self, VALUE layer)
{
	if (SULayerFolderRemoveLayer(FOLDER(self), (SULayerRef){su_unwrap(layer, cLayer)}) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "the tag is not in this folder");
	return Qnil;
}

static VALUE layerfolder_add_folder(VALUE self, VALUE folder)
{
	SULayerFolderRef ref = SU_INVALID;
	if (rb_obj_is_kind_of(folder, cLayerFolder))
		ref.ptr = DATA_PTR(folder);
	else
		SULayerFolderCreate(&ref, StringValueCStr(folder));
	if (SULayerFolderAddLayerFolder(FOLDER(self), ref) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "cannot add this folder");
	return su_wrap(cLayerFolder, ref.ptr);
}

// Removing a folder deletes it from the model.
static VALUE layerfolder_remove_folder(VALUE self, VALUE folder)
{
	VALUE parent = rb_funcall(folder, (rb_intern)("folder"), 0);
	if (!rb_equal(parent, self))
		rb_raise(rb_eArgError, "the folder is not inside this folder");
	SUModelRef model = SU_INVALID;
	SUEntityGetModel(SU_REF(SUEntityRef, self), &model);
	SULayerFolderRef ref = SU_REF(SULayerFolderRef, folder);
	SUModelRemoveLayerFolders(model, 1, &ref);
	return Qnil;
}

static VALUE layerfolder_set_folder(VALUE self, VALUE parent)
{
	if (NIL_P(parent))
		rb_raise(rb_eNotImpError, "the C API cannot move a folder back to the top level");
	if (SULayerFolderAddLayerFolder((SULayerFolderRef){su_unwrap(parent, cLayerFolder)}, FOLDER(self)) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "cannot move the folder there");
	return parent;
}

void Init_layer_folder(void)
{
	rb_include_module(cLayerFolder, rb_mComparable);
	rb_define_method(cLayerFolder, "name", layerfolder_name, 0);
	rb_define_method(cLayerFolder, "name=", layerfolder_set_name, 1);
	rb_define_method(cLayerFolder, "<=>", layerfolder_cmp, 1);
	rb_define_method(cLayerFolder, "visible?", layerfolder_visible_p, 0);
	rb_define_method(cLayerFolder, "visible=", layerfolder_set_visible, 1);
	rb_define_method(cLayerFolder, "folder", layerfolder_folder, 0);
	rb_define_method(cLayerFolder, "folder=", layerfolder_set_folder, 1);
	rb_define_method(cLayerFolder, "layers", layerfolder_layers, 0);
	rb_define_method(cLayerFolder, "folders", layerfolder_folders, 0);
	rb_define_method(cLayerFolder, "each_layer", layerfolder_each_layer, 0);
	rb_define_method(cLayerFolder, "each_folder", layerfolder_each_folder, 0);
	rb_define_method(cLayerFolder, "count_layers", layerfolder_count_layers, 0);
	rb_define_method(cLayerFolder, "count_folders", layerfolder_count_folders, 0);
	rb_define_method(cLayerFolder, "add_layer", layerfolder_add_layer, 1);
	rb_define_method(cLayerFolder, "remove_layer", layerfolder_remove_layer, 1);
	rb_define_method(cLayerFolder, "add_folder", layerfolder_add_folder, 1);
	rb_define_method(cLayerFolder, "remove_folder", layerfolder_remove_folder, 1);
}
