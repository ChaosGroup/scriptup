#include <stdbool.h>
#include <behavior.h>
#include <utils.h>
#include <SketchUpAPI/sketchup.h>

VALUE Behavior_Init(VALUE Sketchup, VALUE Entity)
{
	VALUE Sketchup_Behavior = rb_define_class_under(Sketchup, BEHAVIOR, Entity);
	rb_undef_alloc_func(Sketchup_Behavior);
	return Sketchup_Behavior;
}