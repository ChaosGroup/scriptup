#include <stdbool.h>
#include <entities.h>
#include <SketchUpAPI/sketchup.h>
#include <utils.h>


VALUE Entities_Init(VALUE Sketchup, VALUE rb_cObject)
{
	VALUE Sketchup_Entities = rb_define_class_under(Sketchup, ENTITIES, rb_cObject);
	return Sketchup_Entities;
}