#include <scriptup.h>

#define MODEL(self) SU_REF(SUModelRef, self)

DEFINE_REF_LIST(pages_list, SUModelRef, SUModelGetNumScenes, SUModelGetScenes, SUSceneRef, cPage)

static VALUE pages_to_a(VALUE self) { return pages_list(MODEL(self)); }
static VALUE pages_each(VALUE self) { su_yield_each(self, pages_to_a(self)); return Qnil; }

static VALUE pages_length(VALUE self)
{
	size_t count = 0;
	SUModelGetNumScenes(MODEL(self), &count);
	return SIZET2NUM(count);
}

static VALUE pages_aref(VALUE self, VALUE key)
{
	if (RB_TYPE_P(key, T_STRING))
	{
		SUSceneRef scene = SU_INVALID;
		SUModelGetSceneWithName(MODEL(self), StringValueCStr(key), &scene);
		return su_wrap_or_nil(cPage, scene.ptr);
	}
	return rb_ary_entry(pages_to_a(self), NUM2LONG(key));
}

static VALUE pages_unique_name(VALUE self, VALUE base)
{
	StringValue(base);
	VALUE pages = pages_to_a(self);
	VALUE candidate = base;
	for (long n = 1; !NIL_P(su_find_by(pages, "name", candidate)); n++)
		candidate = rb_sprintf("%"PRIsVALUE" (%ld)", base, n);
	return candidate;
}

static VALUE pages_add(int argc, VALUE* argv, VALUE self)
{
	VALUE name, flags, index;
	rb_scan_args(argc, argv, "03", &name, &flags, &index);
	VALUE unique = pages_unique_name(self, NIL_P(name) ? rb_str_new_cstr("Scene") : name);
	SUSceneRef scene = SU_INVALID;
	SUSceneCreate(&scene);
	SUSceneSetName(scene, StringValueCStr(unique));
	if (!NIL_P(flags))
		SUSceneSetFlags(scene, NUM2UINT(flags));
	int out_index = 0;
	enum SUResult result = SUModelAddScene(MODEL(self), NIL_P(index) ? -1 : NUM2INT(index), scene, &out_index);
	if (result != SU_ERROR_NONE)
	{
		SUSceneRelease(&scene);
		rb_raise(rb_eArgError, "cannot add the page (error %d)", result);
	}
	// Return the model's own scene: on Windows it isn't the ref that was added.
	SUSceneRef added = SU_INVALID;
	SUModelGetSceneWithName(MODEL(self), StringValueCStr(unique), &added);
	return su_wrap(cPage, SUIsValid(added) ? added.ptr : scene.ptr);
}

static VALUE pages_add_matchphoto_page(int argc, VALUE* argv, VALUE self)
{
	VALUE image, camera, name;
	rb_scan_args(argc, argv, "12", &image, &camera, &name);
	SUCameraRef cam = {NIL_P(camera) ? NULL : su_unwrap(camera, cCamera)};
	SUCameraRef created = SU_INVALID;
	if (SUIsInvalid(cam))
	{
		SUCameraCreate(&created);
		cam = created;
	}
	VALUE unique = pages_unique_name(self, NIL_P(name) ? rb_str_new_cstr("Photo") : name);
	SUSceneRef scene = SU_INVALID;
	enum SUResult result = SUModelAddMatchPhotoScene(MODEL(self), StringValueCStr(image), cam, StringValueCStr(unique), &scene);
	if (SUIsValid(created))
		SUCameraRelease(&created);
	if (result != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "cannot add a match photo page from %"PRIsVALUE, image);
	return su_wrap(cPage, scene.ptr);
}

static VALUE pages_erase(VALUE self, VALUE page)
{
	SUSceneRef scene = {su_unwrap(page, cPage)};
	return BOOL2RB(SUModelRemoveScenes(MODEL(self), 1, &scene) == SU_ERROR_NONE);
}

static VALUE pages_reorder(VALUE self, VALUE page, VALUE index)
{
	if (SUModelReorderScene(MODEL(self), (SUSceneRef){su_unwrap(page, cPage)}, NUM2SIZET(index)) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "invalid page index");
	return Qnil;
}

static VALUE pages_selected_page(VALUE self)
{
	SUSceneRef scene = SU_INVALID;
	SUModelGetActiveScene(MODEL(self), &scene);
	return su_wrap_or_nil(cPage, scene.ptr);
}

static VALUE pages_set_selected_page(VALUE self, VALUE page)
{
	return BOOL2RB(SUModelSetActiveScene(MODEL(self), (SUSceneRef){su_unwrap(page, cPage)}) == SU_ERROR_NONE);
}

static VALUE pages_parent(VALUE self)
{
	return su_wrap(cModel, DATA_PTR(self));
}

// Model#pages
static VALUE model_pages(VALUE model)
{
	return su_wrap(cPages, DATA_PTR(model));
}

void Init_pages(void)
{
	rb_include_module(cPages, rb_mEnumerable);
	rb_define_method(cPages, "each", pages_each, 0);
	rb_define_method(cPages, "[]", pages_aref, 1);
	rb_define_method(cPages, "count", pages_length, 0);
	rb_define_method(cPages, "length", pages_length, 0);
	rb_define_method(cPages, "size", pages_length, 0);
	rb_define_method(cPages, "add", pages_add, -1);
	rb_define_method(cPages, "add_matchphoto_page", pages_add_matchphoto_page, -1);
	rb_define_method(cPages, "erase", pages_erase, 1);
	rb_define_method(cPages, "reorder", pages_reorder, 2);
	rb_define_method(cPages, "selected_page", pages_selected_page, 0);
	rb_define_method(cPages, "selected_page=", pages_set_selected_page, 1);
	rb_define_method(cPages, "unique_name", pages_unique_name, 1);
	rb_define_method(cPages, "parent", pages_parent, 0);
	rb_define_method(cModel, "pages", model_pages, 0);
}
