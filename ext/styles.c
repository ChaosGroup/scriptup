#include <scriptup.h>

#define STYLES(self) SU_REF(SUStylesRef, self)

DEFINE_STRING_GETTER(style_name, SUStyleRef, SUStyleGetName)
DEFINE_STRING_SETTER(style_set_name, SUStyleRef, SUStyleSetName)
DEFINE_STRING_GETTER(style_description, SUStyleRef, SUStyleGetDescription)
DEFINE_STRING_SETTER(style_set_description, SUStyleRef, SUStyleSetDescription)
DEFINE_STRING_GETTER(style_path, SUStyleRef, SUStyleGetPath)

DEFINE_REF_LIST(styles_list, SUStylesRef, SUStylesGetNumStyles, SUStylesGetStyles, SUStyleRef, cStyle)
DEFINE_REF_GETTER(styles_active_style, SUStylesRef, SUStylesGetActiveStyle, SUStyleRef, cStyle)
DEFINE_REF_GETTER(styles_selected_style, SUStylesRef, SUStylesGetSelectedStyle, SUStyleRef, cStyle)
DEFINE_BOOL_GETTER(styles_active_style_changed, SUStylesRef, SUStylesGetActiveStyleChanged)

static VALUE styles_to_a(VALUE self) { return styles_list(STYLES(self)); }
static VALUE styles_each(VALUE self) { su_yield_each(self, styles_to_a(self)); return Qnil; }

static VALUE styles_length(VALUE self)
{
	size_t count = 0;
	SUStylesGetNumStyles(STYLES(self), &count);
	return SIZET2NUM(count);
}

static VALUE styles_aref(VALUE self, VALUE key)
{
	VALUE styles = styles_to_a(self);
	if (RB_TYPE_P(key, T_STRING))
		return su_find_by(styles, "name", key);
	return rb_ary_entry(styles, NUM2LONG(key));
}

static VALUE styles_add_style(int argc, VALUE* argv, VALUE self)
{
	VALUE path, select;
	rb_scan_args(argc, argv, "11", &path, &select);
	return BOOL2RB(SUStylesAddStyle(STYLES(self), StringValueCStr(path), RTEST(select)) == SU_ERROR_NONE);
}

static VALUE styles_remove_style(VALUE self, VALUE style)
{
	SUStyleRef ref = {su_unwrap(style, cStyle)};
	if (SUStylesRemoveStyle(STYLES(self), &ref) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "cannot remove this style");
	return Qnil;
}

static VALUE styles_set_selected_style(VALUE self, VALUE style)
{
	SUStylesSetSelectedStyle(STYLES(self), (SUStyleRef){su_unwrap(style, cStyle)});
	return Qnil;
}

static VALUE styles_parent(VALUE self)
{
	SUModelRef model = SU_INVALID;
	SUEntityGetModel(SU_REF(SUEntityRef, self), &model);
	return su_wrap_or_nil(cModel, model.ptr);
}

// Model#styles
static VALUE model_styles(VALUE model)
{
	SUStylesRef styles = SU_INVALID;
	SUModelGetStyles(SU_REF(SUModelRef, model), &styles);
	return su_wrap_or_nil(cStyles, styles.ptr);
}

void Init_styles(void)
{
	rb_define_method(cStyle, "name", style_name, 0);
	rb_define_method(cStyle, "name=", style_set_name, 1);
	rb_define_method(cStyle, "description", style_description, 0);
	rb_define_method(cStyle, "description=", style_set_description, 1);
	rb_define_method(cStyle, "path", style_path, 0);
	rb_include_module(cStyles, rb_mEnumerable);
	rb_define_method(cStyles, "each", styles_each, 0);
	rb_define_method(cStyles, "[]", styles_aref, 1);
	rb_define_method(cStyles, "count", styles_length, 0);
	rb_define_method(cStyles, "length", styles_length, 0);
	rb_define_method(cStyles, "size", styles_length, 0);
	rb_define_method(cStyles, "active_style", styles_active_style, 0);
	rb_define_method(cStyles, "active_style_changed", styles_active_style_changed, 0);
	rb_define_method(cStyles, "selected_style", styles_selected_style, 0);
	rb_define_method(cStyles, "selected_style=", styles_set_selected_style, 1);
	rb_define_method(cStyles, "add_style", styles_add_style, -1);
	rb_define_method(cStyles, "remove_style", styles_remove_style, 1);
	rb_define_method(cStyles, "parent", styles_parent, 0);
	rb_define_method(cModel, "styles", model_styles, 0);
}
