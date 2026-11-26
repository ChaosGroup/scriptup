#include <scriptup.h>

#define COLOR(obj) ((SUColor*)DATA_PTR(obj))

static VALUE color_alloc(VALUE klass)
{
	SUColor* color;
	VALUE obj = Data_Make_Struct(klass, SUColor, 0, RUBY_DEFAULT_FREE, color);
	*color = (SUColor){0, 0, 0, 255};
	return obj;
}

static void color_set_rgba(SUColor* color, int argc, const VALUE* argv)
{
	color->red = (SUByte)su_clamp_int(argv[0], 0, 255);
	color->green = (SUByte)su_clamp_int(argv[1], 0, 255);
	color->blue = (SUByte)su_clamp_int(argv[2], 0, 255);
	color->alpha = argc > 3 ? (SUByte)su_clamp_int(argv[3], 0, 255) : 255;
}

VALUE su_color_new(SUColor value)
{
	VALUE obj = color_alloc(cColor);
	*COLOR(obj) = value;
	return obj;
}

VALUE su_color_coerce(VALUE obj)
{
	if (rb_obj_is_kind_of(obj, cColor))
		return obj;
	if (!RB_TYPE_P(obj, T_ARRAY))
		rb_raise(rb_eTypeError, "no implicit conversion to Color");
	if (RARRAY_LEN(obj) < 3)
		rb_raise(rb_eArgError, "At least 3 values required");
	VALUE color = color_alloc(cColor);
	color_set_rgba(COLOR(color), (int)RARRAY_LEN(obj), RARRAY_CONST_PTR(obj));
	return color;
}

static VALUE color_names(VALUE self)
{
	size_t count = 0;
	SUColorGetNumNames(&count);
	SUStringRef* names = ZALLOC_N(SUStringRef, count);
	for (size_t i = 0; i < count; i++)
	{
		names[i].ptr = NULL;
		SUStringCreate(&names[i]);
	}
	SUColorGetNames(names, count);
	VALUE ary = rb_ary_new_capa((long)count);
	for (size_t i = 0; i < count; i++)
	{
		rb_ary_push(ary, su_string_to_rb(names[i]));
		SUStringRelease(&names[i]);
	}
	xfree(names);
	return ary;
}

static VALUE color_initialize(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 0, 4);
	SUColor* color = COLOR(self);
	if (argc == 1 && RB_TYPE_P(argv[0], T_STRING))
		SUColorSetByName(color, StringValueCStr(argv[0]));
	else if (argc == 1 && RB_INTEGER_TYPE_P(argv[0]))
		SUColorSetByValue(color, NUM2UINT(argv[0]));
	else if (argc >= 3)
		color_set_rgba(color, argc, argv);
	return self;
}

static VALUE color_eq(VALUE self, VALUE other)
{
	if (!rb_obj_is_kind_of(other, cColor))
		return Qfalse;
	SUColor* a = COLOR(self);
	SUColor* b = COLOR(other);
	return BOOL2RB(a->red == b->red && a->green == b->green && a->blue == b->blue && a->alpha == b->alpha);
}

static VALUE color_blend(VALUE self, VALUE other, VALUE weight)
{
	SUColor output;
	SUColorBlend(*COLOR(self), *COLOR(su_color_coerce(other)), NUM2DBL(weight), &output);
	return su_color_new(output);
}

#define DEFINE_CHANNEL(channel) \
	static VALUE color_##channel(VALUE self) { return INT2FIX(COLOR(self)->channel); } \
	static VALUE color_set_##channel(VALUE self, VALUE value) \
	{ \
		COLOR(self)->channel = (SUByte)su_clamp_int(value, 0, 255); \
		return value; \
	}
DEFINE_CHANNEL(red)
DEFINE_CHANNEL(green)
DEFINE_CHANNEL(blue)
DEFINE_CHANNEL(alpha)

static VALUE color_to_a(VALUE self)
{
	SUColor* c = COLOR(self);
	return rb_ary_new_from_args(4, INT2FIX(c->red), INT2FIX(c->green), INT2FIX(c->blue), INT2FIX(c->alpha));
}

static VALUE color_to_i(VALUE self)
{
	SUColor* c = COLOR(self);
	return INT2NUM(c->red | c->green << 8 | c->blue << 16);
}

static VALUE color_to_s(VALUE self)
{
	SUColor* c = COLOR(self);
	return rb_sprintf("Color(%d, %d, %d, %d)", c->red, c->green, c->blue, c->alpha);
}

void Init_color(void)
{
	rb_define_alloc_func(cColor, color_alloc);
	rb_define_singleton_method(cColor, "names", color_names, 0);
	rb_define_method(cColor, "initialize", color_initialize, -1);
	rb_define_method(cColor, "==", color_eq, 1);
	rb_define_method(cColor, "blend", color_blend, 2);
	rb_define_method(cColor, "red", color_red, 0);
	rb_define_method(cColor, "red=", color_set_red, 1);
	rb_define_method(cColor, "green", color_green, 0);
	rb_define_method(cColor, "green=", color_set_green, 1);
	rb_define_method(cColor, "blue", color_blue, 0);
	rb_define_method(cColor, "blue=", color_set_blue, 1);
	rb_define_method(cColor, "alpha", color_alpha, 0);
	rb_define_method(cColor, "alpha=", color_set_alpha, 1);
	rb_define_method(cColor, "to_a", color_to_a, 0);
	rb_define_method(cColor, "to_i", color_to_i, 0);
	rb_define_method(cColor, "to_s", color_to_s, 0);
}
