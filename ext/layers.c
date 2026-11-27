#include <scriptup.h>

#define MODEL(self) SU_REF(SUModelRef, self)

DEFINE_REF_LIST(layers_to_a, SUModelRef, SUModelGetNumLayers, SUModelGetLayers, SULayerRef, cLayer)
DEFINE_REF_LIST(layers_top_level, SUModelRef, SUModelGetNumTopLevelLayers, SUModelGetTopLevelLayers, SULayerRef, cLayer)
DEFINE_REF_LIST(layers_folder_list, SUModelRef, SUModelGetNumLayerFolders, SUModelGetLayerFolders, SULayerFolderRef, cLayerFolder)

static VALUE layers_all(VALUE self) { return layers_to_a(MODEL(self)); }
static VALUE layers_layers(VALUE self) { return layers_top_level(MODEL(self)); }
static VALUE layers_folders(VALUE self) { return layers_folder_list(MODEL(self)); }

static VALUE layers_each(VALUE self) { return su_yield_each(self, layers_all(self)); }
static VALUE layers_each_layer(VALUE self) { return su_yield_each(self, layers_layers(self)); }
static VALUE layers_each_folder(VALUE self) { return su_yield_each(self, layers_folders(self)); }

static VALUE layers_length(VALUE self) { return LONG2NUM(RARRAY_LEN(layers_all(self))); }
static VALUE layers_count_layers(VALUE self) { return LONG2NUM(RARRAY_LEN(layers_layers(self))); }
static VALUE layers_count_folders(VALUE self) { return LONG2NUM(RARRAY_LEN(layers_folders(self))); }

static VALUE layers_aref(VALUE self, VALUE key)
{
	VALUE layers = layers_all(self);
	if (RB_TYPE_P(key, T_STRING))
		return su_find_by(layers, "name", key);
	return rb_ary_entry(layers, NUM2LONG(key));
}

static VALUE layers_unique_name(int argc, VALUE* argv, VALUE self)
{
	VALUE base;
	rb_scan_args(argc, argv, "01", &base);
	base = NIL_P(base) ? rb_str_new_cstr("Tag") : StringValue(base);
	VALUE layers = layers_all(self);
	VALUE candidate = base;
	for (long n = 1; !NIL_P(su_find_by(layers, "name", candidate)); n++)
		candidate = rb_sprintf("%"PRIsVALUE"%ld", base, n);
	return candidate;
}

static VALUE layers_add(VALUE self, VALUE name)
{
	VALUE existing = su_find_by(layers_all(self), "name", StringValue(name));
	if (!NIL_P(existing))
		return existing;
	SULayerRef layer = SU_INVALID;
	SULayerCreate(&layer);
	SULayerSetName(layer, StringValueCStr(name));
	SUModelAddLayers(MODEL(self), 1, &layer);
	return su_wrap(cLayer, layer.ptr);
}

static VALUE layers_add_folder(VALUE self, VALUE folder)
{
	SULayerFolderRef ref = SU_INVALID;
	if (rb_obj_is_kind_of(folder, cLayerFolder))
		ref.ptr = DATA_PTR(folder);
	else
		SULayerFolderCreate(&ref, StringValueCStr(folder));
	SUModelAddLayerFolder(MODEL(self), ref);
	return su_wrap(cLayerFolder, ref.ptr);
}

static VALUE layers_remove(int argc, VALUE* argv, VALUE self)
{
	VALUE layer;
	VALUE remove_geometry;
	rb_scan_args(argc, argv, "11", &layer, &remove_geometry);
	if (RTEST(remove_geometry))
		rb_raise(rb_eNotImpError, "the C API cannot remove a tag's geometry");
	if (!rb_obj_is_kind_of(layer, cLayer))
		layer = layers_aref(self, layer);
	if (NIL_P(layer))
		return Qfalse;
	SULayerRef ref = SU_REF(SULayerRef, layer);
	return BOOL2RB(SUModelRemoveLayers(MODEL(self), 1, &ref) == SU_ERROR_NONE);
}

static VALUE layers_remove_folder(VALUE self, VALUE folder)
{
	SULayerFolderRef ref = {su_unwrap(folder, cLayerFolder)};
	SUModelRemoveLayerFolders(MODEL(self), 1, &ref);
	return Qnil;
}

static VALUE layers_purge_unused(VALUE self)
{
	size_t count = 0;
	SUModelPurgeUnusedLayers(MODEL(self), &count);
	return SIZET2NUM(count);
}

static VALUE layers_purge_unused_folders(VALUE self)
{
	size_t count = 0;
	SUModelPurgeEmptyLayerFolders(MODEL(self), &count);
	return SIZET2NUM(count);
}

void Init_layers(void)
{
	rb_include_module(cLayers, rb_mEnumerable);
	rb_define_method(cLayers, "each", layers_each, 0);
	rb_define_method(cLayers, "each_layer", layers_each_layer, 0);
	rb_define_method(cLayers, "each_folder", layers_each_folder, 0);
	rb_define_method(cLayers, "layers", layers_layers, 0);
	rb_define_method(cLayers, "folders", layers_folders, 0);
	rb_define_method(cLayers, "[]", layers_aref, 1);
	rb_define_method(cLayers, "at", layers_aref, 1);
	rb_define_method(cLayers, "count", layers_length, 0);
	rb_define_method(cLayers, "length", layers_length, 0);
	rb_define_method(cLayers, "size", layers_length, 0);
	rb_define_method(cLayers, "count_layers", layers_count_layers, 0);
	rb_define_method(cLayers, "count_folders", layers_count_folders, 0);
	rb_define_method(cLayers, "add", layers_add, 1);
	rb_define_method(cLayers, "add_folder", layers_add_folder, 1);
	rb_define_method(cLayers, "remove", layers_remove, -1);
	rb_define_method(cLayers, "remove_folder", layers_remove_folder, 1);
	rb_define_method(cLayers, "purge_unused", layers_purge_unused, 0);
	rb_define_method(cLayers, "purge_unused_folders", layers_purge_unused_folders, 0);
	rb_define_method(cLayers, "unique_name", layers_unique_name, -1);
}
