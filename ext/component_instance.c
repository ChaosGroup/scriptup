#include <scriptup.h>

#define INSTANCE(self) SU_REF(SUComponentInstanceRef, self)

// Shared with Sketchup::Group, which is a component instance in the C API.
DEFINE_BOOL_GETTER(su_instance_locked_p_impl, SUComponentInstanceRef, SUComponentInstanceIsLocked)
DEFINE_BOOL_SETTER(su_instance_set_locked_impl, SUComponentInstanceRef, SUComponentInstanceSetLocked)

VALUE su_instance_locked_p(VALUE self) { return su_instance_locked_p_impl(self); }
VALUE su_instance_set_locked(VALUE self, VALUE lock) { return su_instance_set_locked_impl(self, lock); }

VALUE su_instance_volume(VALUE self)
{
	double volume = 0.0;
	if (SUComponentInstanceComputeVolume(INSTANCE(self), NULL, &volume) != SU_ERROR_NONE)
		return DBL2NUM(-1.0);
	return DBL2NUM(volume);
}

VALUE su_instance_manifold_p(VALUE self)
{
	double volume = 0.0;
	return BOOL2RB(SUComponentInstanceComputeVolume(INSTANCE(self), NULL, &volume) == SU_ERROR_NONE);
}

VALUE su_instance_glued_to(VALUE self)
{
	size_t count = 0;
	SUComponentInstanceGetNumAttachedToDrawingElements(INSTANCE(self), &count);
	if (count == 0)
		return Qnil;
	SUDrawingElementRef* elements = ZALLOC_N(SUDrawingElementRef, count);
	SUComponentInstanceGetAttachedToDrawingElements(INSTANCE(self), count, elements, &count);
	VALUE glued = su_wrap_entity((SUEntityRef){elements[0].ptr});
	xfree(elements);
	return glued;
}

DEFINE_STRING_GETTER(componentinstance_name, SUComponentInstanceRef, SUComponentInstanceGetName)
DEFINE_STRING_SETTER(componentinstance_set_name, SUComponentInstanceRef, SUComponentInstanceSetName)
DEFINE_STRING_GETTER(componentinstance_guid, SUComponentInstanceRef, SUComponentInstanceGetGuid)
DEFINE_REF_GETTER(componentinstance_definition, SUComponentInstanceRef, SUComponentInstanceGetDefinition, SUComponentDefinitionRef, cComponentDefinition)

static VALUE componentinstance_transformation(VALUE self)
{
	struct SUTransformation t;
	SUComponentInstanceGetTransform(INSTANCE(self), &t);
	return su_transformation_new(t);
}

static VALUE componentinstance_set_transformation(VALUE self, VALUE transformation)
{
	struct SUTransformation t = su_to_transformation(transformation);
	SUComponentInstanceSetTransform(INSTANCE(self), &t);
	return transformation;
}

static VALUE componentinstance_move_bang(VALUE self, VALUE transformation)
{
	componentinstance_set_transformation(self, transformation);
	return Qtrue;
}

static VALUE componentinstance_transform_bang(VALUE self, VALUE transformation)
{
	struct SUTransformation by = su_to_transformation(transformation);
	struct SUTransformation current;
	struct SUTransformation result;
	SUComponentInstanceGetTransform(INSTANCE(self), &current);
	SUTransformationMultiply(&by, &current, &result);
	SUComponentInstanceSetTransform(INSTANCE(self), &result);
	return Qtrue;
}

void Init_component_instance(void)
{
	rb_define_method(cComponentInstance, "name", componentinstance_name, 0);
	rb_define_method(cComponentInstance, "name=", componentinstance_set_name, 1);
	rb_define_method(cComponentInstance, "guid", componentinstance_guid, 0);
	rb_define_method(cComponentInstance, "definition", componentinstance_definition, 0);
	rb_define_method(cComponentInstance, "transformation", componentinstance_transformation, 0);
	rb_define_method(cComponentInstance, "transformation=", componentinstance_set_transformation, 1);
	rb_define_method(cComponentInstance, "move!", componentinstance_move_bang, 1);
	rb_define_method(cComponentInstance, "transform!", componentinstance_transform_bang, 1);
	rb_define_method(cComponentInstance, "locked?", su_instance_locked_p, 0);
	rb_define_method(cComponentInstance, "locked=", su_instance_set_locked, 1);
	rb_define_method(cComponentInstance, "volume", su_instance_volume, 0);
	rb_define_method(cComponentInstance, "manifold?", su_instance_manifold_p, 0);
	rb_define_method(cComponentInstance, "glued_to", su_instance_glued_to, 0);
}
