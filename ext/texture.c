#include <stdbool.h>
#include <texture.h>
#include <SketchUpAPI/sketchup.h>
#include <utils.h>

static VALUE Sketchup_Texture_average_color(VALUE self)
{
	SUTextureRef texture = {DATA_PTR(self)};
	SUColor* color_val;
	enum SUResult result = SUTextureGetAverageColor(texture, &color_val);
	if (result != SU_ERROR_NONE || !color_val)
		return Qnil;

	return Data_Wrap_Struct(rb_path2class(SKETCHUP_COLOR), 0, 0, color_val);
}

void Texture_Init(VALUE Sketchup, VALUE Sketchup_Entity)
{
	VALUE Sketchup_Texture = rb_define_class_under(Sketchup, TEXTURE, Sketchup_Entity);
	rb_undef_alloc_func(Sketchup_Texture);
	rb_define_method(Sketchup_Texture, "average_color", Sketchup_Texture_average_color, 0);
}
