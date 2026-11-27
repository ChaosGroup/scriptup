#include <scriptup.h>

#define ENVIRONMENT(self) SU_REF(SUEnvironmentRef, self)
#define ENVIRONMENTS(self) SU_REF(SUEnvironmentsRef, self)

DEFINE_STRING_GETTER(environment_name, SUEnvironmentRef, SUEnvironmentGetName)
DEFINE_STRING_SETTER(environment_set_name, SUEnvironmentRef, SUEnvironmentSetName)
DEFINE_STRING_GETTER(environment_description, SUEnvironmentRef, SUEnvironmentGetDescription)
DEFINE_STRING_SETTER(environment_set_description, SUEnvironmentRef, SUEnvironmentSetDescription)
DEFINE_BOOL_GETTER(environment_use_as_skydome_p, SUEnvironmentRef, SUEnvironmentGetUseAsSkydome)
DEFINE_BOOL_SETTER(environment_set_use_as_skydome, SUEnvironmentRef, SUEnvironmentSetUseAsSkydome)
DEFINE_BOOL_GETTER(environment_use_for_reflections_p, SUEnvironmentRef, SUEnvironmentGetUseForReflections)
DEFINE_BOOL_SETTER(environment_set_use_for_reflections, SUEnvironmentRef, SUEnvironmentSetUseForReflections)
DEFINE_BOOL_GETTER(environment_linked_sun_p, SUEnvironmentRef, SUEnvironmentGetLinkedSun)
DEFINE_BOOL_SETTER(environment_set_linked_sun, SUEnvironmentRef, SUEnvironmentSetLinkedSun)
DEFINE_DOUBLE_GETTER(environment_rotation, SUEnvironmentRef, SUEnvironmentGetRotation)
DEFINE_SETTER(environment_set_rotation, SUEnvironmentRef, SUEnvironmentSetRotation, NUM2DBL)
DEFINE_DOUBLE_GETTER(environment_skydome_exposure, SUEnvironmentRef, SUEnvironmentGetSkydomeExposure)
DEFINE_SETTER(environment_set_skydome_exposure, SUEnvironmentRef, SUEnvironmentSetSkydomeExposure, NUM2DBL)
DEFINE_DOUBLE_GETTER(environment_reflection_exposure, SUEnvironmentRef, SUEnvironmentGetReflectionExposure)
DEFINE_SETTER(environment_set_reflection_exposure, SUEnvironmentRef, SUEnvironmentSetReflectionExposure, NUM2DBL)

static VALUE environment_linked_sun_position(VALUE self)
{
	struct SUPoint2D p = {0, 0};
	SUEnvironmentGetLinkedSunPosition(ENVIRONMENT(self), &p);
	return su_point3d_new((struct SUPoint3D){p.x, p.y, 0});
}

static VALUE environment_set_linked_sun_position(VALUE self, VALUE position)
{
	struct SUPoint3D p = su_to_point3d(position);
	struct SUPoint2D xy = {p.x, p.y};
	if (SUEnvironmentSetLinkedSunPosition(ENVIRONMENT(self), &xy) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "invalid sun position");
	return position;
}

static VALUE environment_thumbnail(VALUE self)
{
	SUImageRepRef image = SU_INVALID;
	SUImageRepCreate(&image);
	if (SUEnvironmentGetThumbnail(ENVIRONMENT(self), &image) != SU_ERROR_NONE)
	{
		SUImageRepRelease(&image);
		return Qnil;
	}
	return su_image_rep_new(image);
}

// Writes the HDR image into the given directory and returns the written path.
static VALUE environment_write_hdr(VALUE self, VALUE directory)
{
	SUStringRef path = SU_INVALID;
	SUStringCreate(&path);
	enum SUResult result = SUEnvironmentWriteHDRImageToFile(ENVIRONMENT(self), StringValueCStr(directory), &path);
	VALUE output = su_string_to_rb(path);
	SUStringRelease(&path);
	if (result != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "cannot write the HDR image to %"PRIsVALUE, directory);
	return output;
}

DEFINE_REF_LIST(environments_list, SUEnvironmentsRef, SUEnvironmentsGetCount, SUEnvironmentsGetAll, SUEnvironmentRef, cEnvironment)
DEFINE_SIZE_GETTER(environments_size, SUEnvironmentsRef, SUEnvironmentsGetCount)

static VALUE environments_each(VALUE self)
{
	return su_yield_each(self, environments_list(ENVIRONMENTS(self)));
}

static VALUE environments_aref(VALUE self, VALUE name)
{
	SUEnvironmentRef environment = SU_INVALID;
	SUEnvironmentsGetEnvironmentByName(ENVIRONMENTS(self), StringValueCStr(name), &environment);
	return su_wrap_or_nil(cEnvironment, environment.ptr);
}

static VALUE environments_add(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 1, 2);
	VALUE path = argv[argc - 1];
	const char* name = argc == 2 ? StringValueCStr(argv[0]) : "";
	SUEnvironmentRef environment = SU_INVALID;
	if (SUEnvironmentCreateFromFile(&environment, StringValueCStr(path), name) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "cannot load an environment from %"PRIsVALUE, path);
	if (SUEnvironmentsAdd(ENVIRONMENTS(self), environment) != SU_ERROR_NONE)
	{
		SUEnvironmentRelease(&environment);
		rb_raise(rb_eArgError, "cannot add the environment");
	}
	return su_wrap(cEnvironment, environment.ptr);
}

static VALUE environments_remove(VALUE self, VALUE environment)
{
	return BOOL2RB(SUEnvironmentsRemove(ENVIRONMENTS(self), (SUEnvironmentRef){su_unwrap(environment, cEnvironment)}) == SU_ERROR_NONE);
}

static VALUE environments_current(VALUE self)
{
	SUEnvironmentRef environment = SU_INVALID;
	SUEnvironmentsGetSelectedEnvironment(ENVIRONMENTS(self), &environment);
	return su_wrap_or_nil(cEnvironment, environment.ptr);
}

// Model#environments
static VALUE model_environments(VALUE model)
{
	SUEnvironmentsRef environments = SU_INVALID;
	SUModelGetEnvironments(SU_REF(SUModelRef, model), &environments);
	return su_wrap_or_nil(cEnvironments, environments.ptr);
}

void Init_environments(void)
{
	static const struct { const char* name; VALUE (*fn)(ANYARGS); int argc; } methods[] = {
		{"name", RUBY_METHOD_FUNC(environment_name), 0},
		{"name=", RUBY_METHOD_FUNC(environment_set_name), 1},
		{"description", RUBY_METHOD_FUNC(environment_description), 0},
		{"description=", RUBY_METHOD_FUNC(environment_set_description), 1},
		{"use_as_skydome?", RUBY_METHOD_FUNC(environment_use_as_skydome_p), 0},
		{"use_as_skydome=", RUBY_METHOD_FUNC(environment_set_use_as_skydome), 1},
		{"use_for_reflections?", RUBY_METHOD_FUNC(environment_use_for_reflections_p), 0},
		{"use_for_reflections=", RUBY_METHOD_FUNC(environment_set_use_for_reflections), 1},
		{"linked_sun?", RUBY_METHOD_FUNC(environment_linked_sun_p), 0},
		{"linked_sun=", RUBY_METHOD_FUNC(environment_set_linked_sun), 1},
		{"linked_sun_position", RUBY_METHOD_FUNC(environment_linked_sun_position), 0},
		{"linked_sun_position=", RUBY_METHOD_FUNC(environment_set_linked_sun_position), 1},
		{"rotation", RUBY_METHOD_FUNC(environment_rotation), 0},
		{"rotation=", RUBY_METHOD_FUNC(environment_set_rotation), 1},
		{"skydome_exposure", RUBY_METHOD_FUNC(environment_skydome_exposure), 0},
		{"skydome_exposure=", RUBY_METHOD_FUNC(environment_set_skydome_exposure), 1},
		{"reflection_exposure", RUBY_METHOD_FUNC(environment_reflection_exposure), 0},
		{"reflection_exposure=", RUBY_METHOD_FUNC(environment_set_reflection_exposure), 1},
		{"thumbnail", RUBY_METHOD_FUNC(environment_thumbnail), 0},
		{"write_hdr", RUBY_METHOD_FUNC(environment_write_hdr), 1},
	};
	for (size_t i = 0; i < sizeof(methods) / sizeof(methods[0]); i++)
		rb_define_method(cEnvironment, methods[i].name, methods[i].fn, methods[i].argc);
	rb_include_module(cEnvironments, rb_mEnumerable);
	rb_define_method(cEnvironments, "each", environments_each, 0);
	rb_define_method(cEnvironments, "[]", environments_aref, 1);
	rb_define_method(cEnvironments, "size", environments_size, 0);
	rb_define_method(cEnvironments, "add", environments_add, -1);
	rb_define_method(cEnvironments, "remove", environments_remove, 1);
	rb_define_method(cEnvironments, "current", environments_current, 0);
	rb_define_method(cModel, "environments", model_environments, 0);
}
