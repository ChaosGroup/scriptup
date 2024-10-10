#include <stdbool.h>
#include <color.h>
#include <SketchUpAPI/sketchup.h>
#include <utils.h>

static int GETCOLOR(VALUE obj)
{
	int color = FIX2INT(obj);
	if (color > 255)
		return 255;
	else if (color < 0)
		return 0;
	else
		return color;
}

static float GETALPHA(VALUE obj)
{
	float alpha = NUM2DBL(obj);
	if (alpha > 1.0)
		return 1.0;
	else if (alpha < 0.0)
		return 0.0;
	else
		return alpha;
}

static VALUE alloc(VALUE self)
{
	SUColor* color;
	VALUE obj = Data_Make_Struct(self, SUColor, 0, RUBY_DEFAULT_FREE, color);
	color->red = 0;
	color->green = 0;
	color->blue = 0;
	color->alpha = 255;
	return obj;
}

static VALUE Sketchup_Color_names(VALUE self)
{
	size_t count = 0;
	SUColorGetNumNames(&count);
	SUStringRef* names = malloc(count * sizeof(SUStringRef));
	for (int i = 0; i < count; i++)
	{
		names[i].ptr = 0;
		SUStringCreate(&names[i]);
	}
	SUColorGetNames(names, count);
	VALUE ary = rb_ary_new2(count);
	for (int i = 0; i < count; i++)
	{
		VALUE name;
		GETUTF8FROMSTRING(names[i], name);
		rb_ary_push(ary, name);
	}
	for (int i = 0; i < count; i++)
	{
		SUStringRelease(&names[i]);
	}
	free(names);
	return ary;
}

static VALUE Sketchup_Color_initialize(int argc, VALUE* argv, VALUE self)
{
	SUColor* color = DATA_PTR(self);
	if (argc == 1)
	{
		if (rb_type(argv[0]) == T_STRING) // && check if string is part of the list
		{
			SUColorSetByName(color, StringValuePtr(argv[0]));
		}
		else if (rb_type(argv[0]) == T_FIXNUM) // && check if its hex
		{
			SUColorSetByValue(color, NUM2INT(argv[0]));
		}
	}
	else if (argc == 3)
	{
		color->red = GETCOLOR(argv[0]);
		color->green = GETCOLOR(argv[1]);
		color->blue = GETCOLOR(argv[2]);
		color->alpha = 255;
	}
	else if (argc == 4)
	{
		color->red = GETCOLOR(argv[0]);
		color->green = GETCOLOR(argv[1]);
		color->blue = GETCOLOR(argv[2]);
		color->alpha = GETALPHA(argv[3]);
	}
	return self;
}

static VALUE Sketchup_Color_equ(VALUE self, VALUE other)
{
	SUColor* color = DATA_PTR(self);
	SUColor* otherColor = DATA_PTR(other);
	return (color->red == otherColor->red && color->green == otherColor->green && color->blue == otherColor->blue && color->alpha == otherColor->alpha)
		? Qtrue
		: Qfalse;
}

static VALUE Sketchup_Color_Get_alpha(VALUE self)
{
	SUColor* color = DATA_PTR(self);
	return INT2NUM(color->alpha);
}

static VALUE Sketchup_Color_Set_alpha(VALUE self, VALUE alpha)
{
	SUColor* color = DATA_PTR(self);
	color->alpha = NUM2INT(alpha);
	return alpha;
}

static VALUE Sketchup_Color_blend(VALUE self, VALUE other, VALUE weight)
{
	SUColor* color = DATA_PTR(self);
	SUColor* otherColor = DATA_PTR(other);
	VALUE output = rb_obj_alloc(rb_path2class(SKETCHUP_COLOR));
	SUColor* outputColor = DATA_PTR(output);
	SUColorBlend(*color, *otherColor, NUM2DBL(weight), outputColor);
	return output;
}

static VALUE Sketchup_Color_Get_blue(VALUE self)
{
	SUColor* color = DATA_PTR(self);
	return INT2NUM(color->blue);
}

static VALUE Sketchup_Color_Set_blue(VALUE self, VALUE blue)
{
	SUColor* color = DATA_PTR(self);
	color->blue = NUM2INT(blue);
	return blue;
}

static VALUE Sketchup_Color_Get_green(VALUE self)
{
	SUColor* color = DATA_PTR(self);
	return INT2NUM(color->green);
}

static VALUE Sketchup_Color_Set_green(VALUE self, VALUE green)
{
	SUColor* color = DATA_PTR(self);
	color->green = NUM2INT(green);
	return green;
}

static VALUE Sketchup_Color_Get_red(VALUE self)
{
	SUColor* color = DATA_PTR(self);
	return INT2NUM(color->red);
}

static VALUE Sketchup_Color_Set_red(VALUE self, VALUE red)
{
	SUColor* color = DATA_PTR(self);
	color->red = NUM2INT(red);
	return red;
}

static VALUE Sketchup_Color_to_a(VALUE self)
{
	SUColor* color = DATA_PTR(self);
	VALUE ary = rb_ary_new2(4);
	rb_ary_push(ary, INT2NUM(color->red));
	rb_ary_push(ary, INT2NUM(color->green));
	rb_ary_push(ary, INT2NUM(color->blue));
	rb_ary_push(ary, INT2NUM(color->alpha));
	return ary;
}

static VALUE Sketchup_Color_to_i(VALUE self)
{
	SUColor* color = DATA_PTR(self);
	return INT2NUM(color->red | color->green << 8 | color->blue << 16);
}

static VALUE Sketchup_Color_to_s(VALUE self)
{
	SUColor* color = DATA_PTR(self);
	VALUE output;
	output = rb_sprintf("Color(%d, %d, %d, %d)", color->red, color->green, color->blue, color->alpha);
	return output;
}

void Color_Init(VALUE Sketchup, VALUE Sketchup_Object)
{
	VALUE Sketchup_Color = rb_define_class_under(Sketchup, COLOR, Sketchup_Object);
	rb_define_alloc_func(Sketchup_Color, alloc);
	rb_define_singleton_method(Sketchup_Color, "names", Sketchup_Color_names, 0);
	rb_define_method(Sketchup_Color, "initialize", Sketchup_Color_initialize, -1);
	rb_define_method(Sketchup_Color, "==", Sketchup_Color_equ, 1);
	rb_define_method(Sketchup_Color, "alpha", Sketchup_Color_Get_alpha, 0);
	rb_define_method(Sketchup_Color, "alpha=", Sketchup_Color_Set_alpha, 1);
	rb_define_method(Sketchup_Color, "blend", Sketchup_Color_blend, 2);
	rb_define_method(Sketchup_Color, "blue", Sketchup_Color_Get_blue, 0);
	rb_define_method(Sketchup_Color, "blue=", Sketchup_Color_Set_blue, 1);
	rb_define_method(Sketchup_Color, "green", Sketchup_Color_Get_green, 0);
	rb_define_method(Sketchup_Color, "green=", Sketchup_Color_Set_green, 1);
	rb_define_method(Sketchup_Color, "red", Sketchup_Color_Get_red, 0);
	rb_define_method(Sketchup_Color, "red=", Sketchup_Color_Set_red, 1);
	rb_define_method(Sketchup_Color, "to_a", Sketchup_Color_to_a, 0);
	rb_define_method(Sketchup_Color, "to_i", Sketchup_Color_to_i, 0);
	rb_define_method(Sketchup_Color, "to_s", Sketchup_Color_to_s, 0);
}