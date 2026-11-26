#include <scriptup.h>

DEFINE_STRING_GETTER(componentdefinition_name, SUComponentDefinitionRef, SUComponentDefinitionGetName)
DEFINE_STRING_SETTER(componentdefinition_set_name, SUComponentDefinitionRef, SUComponentDefinitionSetName)
DEFINE_STRING_GETTER(componentdefinition_description, SUComponentDefinitionRef, SUComponentDefinitionGetDescription)
DEFINE_STRING_SETTER(componentdefinition_set_description, SUComponentDefinitionRef, SUComponentDefinitionSetDescription)
DEFINE_STRING_GETTER(componentdefinition_path, SUComponentDefinitionRef, SUComponentDefinitionGetPath)
DEFINE_STRING_GETTER(componentdefinition_guid, SUComponentDefinitionRef, SUComponentDefinitionGetGuid)
DEFINE_SIZE_GETTER(componentdefinition_count_instances, SUComponentDefinitionRef, SUComponentDefinitionGetNumInstances)
DEFINE_SIZE_GETTER(componentdefinition_count_used_instances, SUComponentDefinitionRef, SUComponentDefinitionGetNumUsedInstances)
DEFINE_BOOL_GETTER(componentdefinition_live_component_p, SUComponentDefinitionRef, SUComponentDefinitionIsLiveComponent)
DEFINE_BOOL_GETTER(componentdefinition_internal_p, SUComponentDefinitionRef, SUComponentDefinitionIsInternal)
DEFINE_REF_GETTER(componentdefinition_entities, SUComponentDefinitionRef, SUComponentDefinitionGetEntities, SUEntitiesRef, cEntities)
DEFINE_REF_LIST(componentdefinition_instance_list, SUComponentDefinitionRef, SUComponentDefinitionGetNumInstances,
	SUComponentDefinitionGetInstances, SUComponentInstanceRef, cComponentInstance)

static VALUE componentdefinition_instances(VALUE self)
{
	return componentdefinition_instance_list(SU_REF(SUComponentDefinitionRef, self));
}

static VALUE componentdefinition_cmp(VALUE self, VALUE other)
{
	if (!rb_obj_is_kind_of(other, cComponentDefinition))
		return Qnil;
	return INT2FIX(rb_str_cmp(componentdefinition_name(self), componentdefinition_name(other)));
}

static VALUE componentdefinition_eq(VALUE self, VALUE other)
{
	return BOOL2RB(rb_obj_is_kind_of(other, cComponentDefinition) && DATA_PTR(self) == DATA_PTR(other));
}

static VALUE componentdefinition_group_p(VALUE self)
{
	enum SUComponentType type = SUComponentType_Normal;
	SUComponentDefinitionGetType(SU_REF(SUComponentDefinitionRef, self), &type);
	return BOOL2RB(type == SUComponentType_Group);
}

static VALUE componentdefinition_image_p(VALUE self)
{
	SUDrawingElementRef element = SUComponentDefinitionToDrawingElement(SU_REF(SUComponentDefinitionRef, self));
	return BOOL2RB(SUIsValid(SUImageFromDrawingElement(element)));
}

static VALUE componentdefinition_behavior(VALUE self)
{
	struct SUComponentBehavior* behavior;
	VALUE obj = Data_Make_Struct(cBehavior, struct SUComponentBehavior, 0, RUBY_DEFAULT_FREE, behavior);
	SUComponentDefinitionGetBehavior(SU_REF(SUComponentDefinitionRef, self), behavior);
	return obj;
}

static VALUE componentdefinition_save_as(int argc, VALUE* argv, VALUE self)
{
	VALUE path;
	VALUE version;
	rb_scan_args(argc, argv, "11", &path, &version);
	enum SUModelVersion model_version = NIL_P(version) ? SUModelVersion_Current : (enum SUModelVersion)NUM2INT(version);
	enum SUResult result = SUComponentDefinitionSaveToFile(SU_REF(SUComponentDefinitionRef, self), StringValueCStr(path), model_version);
	return result == SU_ERROR_NONE ? Qtrue : Qnil;
}

// Always the origin since SketchUp 2020; there is no working setter in the C API.
static VALUE componentdefinition_insertion_point(VALUE self)
{
	struct SUPoint3D point = {0, 0, 0};
	SUComponentDefinitionGetInsertPoint(SU_REF(SUComponentDefinitionRef, self), &point);
	return su_point3d_new(point);
}

// The time the definition was loaded from file; the epoch if it wasn't.
static VALUE componentdefinition_load_time(VALUE self)
{
	struct tm time;
	memset(&time, 0, sizeof(time));
	if (SUComponentDefinitionGetLoadTime(SU_REF(SUComponentDefinitionRef, self), &time) != SU_ERROR_NONE)
		return rb_time_new(0, 0);
	return rb_funcall(rb_cTime, (rb_intern)("local"), 6, INT2FIX(time.tm_year + 1900), INT2FIX(time.tm_mon + 1),
		INT2FIX(time.tm_mday), INT2FIX(time.tm_hour), INT2FIX(time.tm_min), INT2FIX(time.tm_sec));
}

void Init_component_definition(void)
{
	rb_include_module(cComponentDefinition, rb_mComparable);
	rb_define_method(cComponentDefinition, "name", componentdefinition_name, 0);
	rb_define_method(cComponentDefinition, "name=", componentdefinition_set_name, 1);
	rb_define_method(cComponentDefinition, "==", componentdefinition_eq, 1);
	rb_define_method(cComponentDefinition, "<=>", componentdefinition_cmp, 1);
	rb_define_method(cComponentDefinition, "instances", componentdefinition_instances, 0);
	rb_define_method(cComponentDefinition, "count_instances", componentdefinition_count_instances, 0);
	rb_define_method(cComponentDefinition, "count_used_instances", componentdefinition_count_used_instances, 0);
	rb_define_method(cComponentDefinition, "live_component?", componentdefinition_live_component_p, 0);
	rb_define_method(cComponentDefinition, "internal?", componentdefinition_internal_p, 0);
	rb_define_method(cComponentDefinition, "entities", componentdefinition_entities, 0);
	rb_define_method(cComponentDefinition, "path", componentdefinition_path, 0);
	rb_define_method(cComponentDefinition, "guid", componentdefinition_guid, 0);
	rb_define_method(cComponentDefinition, "group?", componentdefinition_group_p, 0);
	rb_define_method(cComponentDefinition, "description", componentdefinition_description, 0);
	rb_define_method(cComponentDefinition, "description=", componentdefinition_set_description, 1);
	rb_define_method(cComponentDefinition, "image?", componentdefinition_image_p, 0);
	rb_define_method(cComponentDefinition, "behavior", componentdefinition_behavior, 0);
	rb_define_method(cComponentDefinition, "save_as", componentdefinition_save_as, -1);
	rb_define_method(cComponentDefinition, "save_copy", componentdefinition_save_as, -1);
	rb_define_method(cComponentDefinition, "insertion_point", componentdefinition_insertion_point, 0);
	rb_define_method(cComponentDefinition, "load_time", componentdefinition_load_time, 0);
}
