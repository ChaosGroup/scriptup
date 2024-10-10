#include <stdbool.h>
#include <drawing_element.h>
#include <utils.h>
#include <SketchUpAPI/sketchup.h>

VALUE DrawingElement_Init(VALUE Sketchup, VALUE Sketchup_Entity)
{
	VALUE Sketchup_DrawingElement = rb_define_class_under(Sketchup, DRAWINGELEMENT, Sketchup_Entity);
	return Sketchup_DrawingElement;
}