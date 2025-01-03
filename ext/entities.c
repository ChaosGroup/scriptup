#include <stdbool.h>
#include <entities.h>
#include <SketchUpAPI/sketchup.h>
#include <utils.h>


VALUE Entities_Init(VALUE Sketchup, VALUE rb_cObject)
{
	VALUE Sketchup_Entities = rb_define_class_under(Sketchup, ENTITIES, rb_cObject);
	rb_undef_alloc_func(Sketchup_Entities);
	return Sketchup_Entities;
}