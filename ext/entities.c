#include <stdbool.h>
#include <entities.h>
#include <SketchUpAPI/sketchup.h>
#include <utils.h>

void Sketchup_Entities_Iterator(SUFaceRef face, void* _)
{
	rb_yield(Data_Wrap_Struct(rb_path2class(SKETCHUP_FACE), 0, 0, face.ptr));
}

static VALUE Sketchup_Entities_each(VALUE self)
{
	SUEntitiesRef entities = {DATA_PTR(self)};
	FOREACH(SUEntitiesGetNumFaces, SUEntitiesGetFaces, SUFaceRef, entities, Sketchup_Entities_Iterator, 0);
	return self;
}

VALUE Entities_Init(VALUE Sketchup, VALUE rb_cObject)
{
	VALUE Sketchup_Entities = rb_define_class_under(Sketchup, ENTITIES, rb_cObject);
	rb_undef_alloc_func(Sketchup_Entities);
	rb_include_module(Sketchup_Entities, rb_mEnumerable);
	rb_define_method(Sketchup_Entities, "each", Sketchup_Entities_each, 0);
	return Sketchup_Entities;
}