#include <stdbool.h>
#include <behavior.h>
#include <utils.h>
#include <SketchUpAPI/sketchup.h>

VALUE Behavior_Init(VALUE Sketchup, VALUE Entity)
{
	VALUE Sketchup_Behavior = rb_define_class_under(Sketchup, BEHAVIOR, Entity);
	return Sketchup_Behavior;
}