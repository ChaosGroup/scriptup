#include <stdbool.h>
#include <texture.h>
#include <SketchUpAPI/sketchup.h>
#include <utils.h>

void Texture_Init(VALUE Sketchup, VALUE Sketchup_Entity)
{
	VALUE Sketchup_Texture = rb_define_class_under(Sketchup, TEXTURE, Sketchup_Entity);
}
