#include <scriptup.h>

#define STYLES(self) SU_REF(SULineStylesRef, self)

DEFINE_STRING_GETTER(linestyle_name, SULineStyleRef, SULineStyleGetName)
DEFINE_STRING_LIST(linestyles_name_list, SULineStylesRef, SULineStylesGetNumLineStyles, SULineStylesGetLineStyleNames)

static VALUE linestyles_names(VALUE self)
{
	return linestyles_name_list(STYLES(self));
}

static VALUE linestyles_by_name(VALUE self, VALUE name)
{
	SULineStyleRef style = SU_INVALID;
	if (SULineStylesGetLineStyleByName(STYLES(self), StringValueCStr(name), &style) != SU_ERROR_NONE)
		return Qnil;
	return su_wrap_or_nil(cLineStyle, style.ptr);
}

static VALUE linestyles_to_a(VALUE self)
{
	VALUE names = linestyles_names(self);
	VALUE styles = rb_ary_new_capa(RARRAY_LEN(names));
	for (long i = 0; i < RARRAY_LEN(names); i++)
		rb_ary_push(styles, linestyles_by_name(self, RARRAY_AREF(names, i)));
	return styles;
}

static VALUE linestyles_aref(VALUE self, VALUE key)
{
	if (RB_TYPE_P(key, T_STRING))
		return linestyles_by_name(self, key);
	return rb_ary_entry(linestyles_to_a(self), NUM2LONG(key));
}

static VALUE linestyles_each(VALUE self)
{
	su_yield_each(self, linestyles_to_a(self));
	return Qnil;
}

static VALUE linestyles_size(VALUE self)
{
	size_t count = 0;
	SULineStylesGetNumLineStyles(STYLES(self), &count);
	return SIZET2NUM(count);
}

void Init_line_styles(void)
{
	rb_define_method(cLineStyle, "name", linestyle_name, 0);
	rb_include_module(cLineStyles, rb_mEnumerable);
	rb_define_method(cLineStyles, "names", linestyles_names, 0);
	rb_define_method(cLineStyles, "to_a", linestyles_to_a, 0);
	rb_define_method(cLineStyles, "[]", linestyles_aref, 1);
	rb_define_method(cLineStyles, "at", linestyles_aref, 1);
	rb_define_method(cLineStyles, "each", linestyles_each, 0);
	rb_define_method(cLineStyles, "size", linestyles_size, 0);
}
