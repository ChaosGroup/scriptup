#include <stdbool.h>
#include <component_instance.h>
#include <utils.h>
#include <SketchUpAPI/sketchup.h>

VALUE ComponentInstance_Init(VALUE Sketchup, VALUE Sketchup_DrawingElement)
{
	VALUE Sketchup_ComponentInstance = rb_define_class_under(Sketchup, COMPONENTINSTANCE, Sketchup_DrawingElement);
	rb_undef_alloc_func(Sketchup_ComponentInstance);
	return Sketchup_ComponentInstance;
}