#include <scriptup.h>

#define GROUP(self) SU_REF(SUGroupRef, self)
#define INSTANCE(self) SU_REF(SUComponentInstanceRef, self)

DEFINE_STRING_GETTER(group_name, SUGroupRef, SUGroupGetName)
DEFINE_STRING_SETTER(group_set_name, SUGroupRef, SUGroupSetName)
DEFINE_STRING_GETTER(group_guid, SUGroupRef, SUGroupGetGuid)
DEFINE_REF_GETTER(group_definition, SUGroupRef, SUGroupGetDefinition, SUComponentDefinitionRef, cComponentDefinition)
DEFINE_BOOL_GETTER(group_locked_p, SUComponentInstanceRef, SUComponentInstanceIsLocked)
DEFINE_BOOL_SETTER(group_set_locked, SUComponentInstanceRef, SUComponentInstanceSetLocked)

static VALUE group_entities(VALUE self)
{
	SUEntitiesRef entities = SU_INVALID;
	SUGroupGetEntities(GROUP(self), &entities);
	return su_entities_new(entities);
}

static VALUE group_description(VALUE self)
{
	return rb_funcall(group_definition(self), (rb_intern)("description"), 0);
}

static VALUE group_set_description(VALUE self, VALUE description)
{
	return rb_funcall(group_definition(self), (rb_intern)("description="), 1, description);
}

static VALUE group_transformation(VALUE self)
{
	struct SUTransformation t;
	SUGroupGetTransform(GROUP(self), &t);
	return su_transformation_new(t);
}

static VALUE group_set_transformation(VALUE self, VALUE transformation)
{
	struct SUTransformation t = su_to_transformation(transformation);
	SUGroupSetTransform(GROUP(self), &t);
	return transformation;
}

static VALUE group_move_bang(VALUE self, VALUE transformation)
{
	group_set_transformation(self, transformation);
	return self;
}

static VALUE group_transform_bang(VALUE self, VALUE transformation)
{
	struct SUTransformation by = su_to_transformation(transformation);
	struct SUTransformation current;
	struct SUTransformation result;
	SUGroupGetTransform(GROUP(self), &current);
	SUTransformationMultiply(&by, &current, &result);
	SUGroupSetTransform(GROUP(self), &result);
	return self;
}

static VALUE group_volume(VALUE self)
{
	double volume = 0.0;
	if (SUComponentInstanceComputeVolume(INSTANCE(self), NULL, &volume) != SU_ERROR_NONE)
		return DBL2NUM(-1.0);
	return DBL2NUM(volume);
}

static VALUE group_manifold_p(VALUE self)
{
	double volume = 0.0;
	return BOOL2RB(SUComponentInstanceComputeVolume(INSTANCE(self), NULL, &volume) == SU_ERROR_NONE);
}

static VALUE group_local_bounds(VALUE self)
{
	SUEntitiesRef entities = SU_INVALID;
	SUGroupGetEntities(GROUP(self), &entities);
	struct SUBoundingBox3D bbox;
	SUEntitiesGetBoundingBox(entities, &bbox);
	return su_bounding_box_new(bbox);
}

static VALUE group_glued_to(VALUE self)
{
	size_t count = 0;
	SUComponentInstanceGetNumAttachedToDrawingElements(INSTANCE(self), &count);
	if (count == 0)
		return Qnil;
	SUDrawingElementRef* elements = ALLOC_N(SUDrawingElementRef, count);
	SUComponentInstanceGetAttachedToDrawingElements(INSTANCE(self), count, elements, &count);
	VALUE glued = su_wrap_entity((SUEntityRef){elements[0].ptr});
	xfree(elements);
	return glued;
}

void Init_group(void)
{
	rb_define_method(cGroup, "name", group_name, 0);
	rb_define_method(cGroup, "name=", group_set_name, 1);
	rb_define_method(cGroup, "guid", group_guid, 0);
	rb_define_method(cGroup, "definition", group_definition, 0);
	rb_define_method(cGroup, "entities", group_entities, 0);
	rb_define_method(cGroup, "description", group_description, 0);
	rb_define_method(cGroup, "description=", group_set_description, 1);
	rb_define_method(cGroup, "locked?", group_locked_p, 0);
	rb_define_method(cGroup, "locked=", group_set_locked, 1);
	rb_define_method(cGroup, "transformation", group_transformation, 0);
	rb_define_method(cGroup, "transformation=", group_set_transformation, 1);
	rb_define_method(cGroup, "move!", group_move_bang, 1);
	rb_define_method(cGroup, "transform!", group_transform_bang, 1);
	rb_define_method(cGroup, "volume", group_volume, 0);
	rb_define_method(cGroup, "manifold?", group_manifold_p, 0);
	rb_define_method(cGroup, "local_bounds", group_local_bounds, 0);
	rb_define_method(cGroup, "glued_to", group_glued_to, 0);
}
