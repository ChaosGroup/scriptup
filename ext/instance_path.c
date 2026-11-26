#include <scriptup.h>

#define PATH(self) SU_REF(SUInstancePathRef, self)

static void instancepath_free(void* ptr)
{
	SUInstancePathRef path = {ptr};
	if (ptr)
		SUInstancePathRelease(&path);
}

static VALUE instancepath_alloc(VALUE klass)
{
	return Data_Wrap_Struct(klass, 0, instancepath_free, NULL);
}

// Wraps a copy of a path the C API owns.
VALUE su_instance_path_copy(SUInstancePathRef source)
{
	SUInstancePathRef copy = SU_INVALID;
	if (SUInstancePathCreateCopy(&copy, source) != SU_ERROR_NONE)
		return Qnil;
	return Data_Wrap_Struct(cInstancePath, 0, instancepath_free, copy.ptr);
}

// Accepts an InstancePath or an array of entities: instances, optionally ending in a leaf.
SUInstancePathRef su_to_instance_path(VALUE obj)
{
	if (rb_obj_is_kind_of(obj, cInstancePath))
		return PATH(obj);
	VALUE path = rb_class_new_instance(1, &obj, cInstancePath);
	return PATH(path);
}

static VALUE instancepath_initialize(VALUE self, VALUE entities)
{
	Check_Type(entities, T_ARRAY);
	SUInstancePathRef path = SU_INVALID;
	SUInstancePathCreate(&path);
	DATA_PTR(self) = path.ptr;
	long count = RARRAY_LEN(entities);
	for (long i = 0; i < count; i++)
	{
		VALUE entity = RARRAY_AREF(entities, i);
		su_unwrap(entity, cEntity);
		bool instance = rb_obj_is_kind_of(entity, cComponentInstance) || rb_obj_is_kind_of(entity, cGroup);
		if (instance && i < count - 1)
			SUInstancePathPushInstance(path, (SUComponentInstanceRef){DATA_PTR(entity)});
		else if (i == count - 1)
			SUInstancePathSetLeaf(path, (SUEntityRef){DATA_PTR(entity)});
		else
			rb_raise(rb_eArgError, "only the last entity of a path can be a leaf");
	}
	return self;
}

static VALUE instancepath_length(VALUE self)
{
	size_t depth = 0;
	SUInstancePathGetFullDepth(PATH(self), &depth);
	return SIZET2NUM(depth);
}

static VALUE instancepath_leaf(VALUE self)
{
	SUEntityRef leaf = SU_INVALID;
	SUInstancePathGetLeafAsEntity(PATH(self), &leaf);
	return su_wrap_entity(leaf);
}

static VALUE instancepath_to_a(VALUE self)
{
	size_t depth = 0;
	SUInstancePathGetPathDepth(PATH(self), &depth);
	VALUE entities = rb_ary_new();
	for (size_t i = 0; i < depth; i++)
	{
		SUComponentInstanceRef instance = SU_INVALID;
		SUInstancePathGetInstanceAtDepth(PATH(self), i, &instance);
		rb_ary_push(entities, su_wrap_entity((SUEntityRef){instance.ptr}));
	}
	VALUE leaf = instancepath_leaf(self);
	if (!NIL_P(leaf))
		rb_ary_push(entities, leaf);
	return entities;
}

static VALUE instancepath_aref(VALUE self, VALUE index)
{
	VALUE entities = instancepath_to_a(self);
	long i = NUM2LONG(index);
	if (i < 0 || i >= RARRAY_LEN(entities))
		rb_raise(rb_eIndexError, "index %ld out of range", i);
	return RARRAY_AREF(entities, i);
}

static VALUE instancepath_each(VALUE self)
{
	su_yield_each(self, instancepath_to_a(self));
	return Qnil;
}

static VALUE instancepath_root(VALUE self)
{
	size_t depth = 0;
	SUInstancePathGetPathDepth(PATH(self), &depth);
	if (depth == 0)
		return Qnil;
	SUComponentInstanceRef instance = SU_INVALID;
	SUInstancePathGetInstanceAtDepth(PATH(self), 0, &instance);
	return su_wrap_entity((SUEntityRef){instance.ptr});
}

static VALUE instancepath_empty_p(VALUE self)
{
	bool empty = true;
	SUInstancePathIsEmpty(PATH(self), &empty);
	return BOOL2RB(empty);
}

static VALUE instancepath_valid_p(VALUE self)
{
	bool valid = false;
	SUInstancePathIsValid(PATH(self), &valid);
	return BOOL2RB(valid);
}

static VALUE instancepath_include_p(VALUE self, VALUE entity)
{
	if (!rb_obj_is_kind_of(entity, cEntity))
		return Qfalse;
	bool contains = false;
	SUInstancePathContains(PATH(self), (SUEntityRef){DATA_PTR(entity)}, &contains);
	return BOOL2RB(contains);
}

static VALUE instancepath_persistent_id_path(VALUE self)
{
	SUStringRef string = SU_INVALID;
	SUStringCreate(&string);
	SUInstancePathGetPersistentID(PATH(self), &string);
	VALUE output = su_string_to_rb(string);
	SUStringRelease(&string);
	return output;
}

static VALUE instancepath_transformation(int argc, VALUE* argv, VALUE self)
{
	VALUE index;
	rb_scan_args(argc, argv, "01", &index);
	struct SUTransformation t;
	enum SUResult result = NIL_P(index)
		? SUInstancePathGetTransform(PATH(self), &t)
		: SUInstancePathGetTransformAtDepth(PATH(self), NUM2SIZET(index), &t);
	if (result != SU_ERROR_NONE)
		rb_raise(rb_eIndexError, "invalid path depth");
	return su_transformation_new(t);
}

static VALUE instancepath_eq(VALUE self, VALUE other)
{
	if (!rb_obj_is_kind_of(other, cInstancePath))
		return Qfalse;
	return rb_equal(instancepath_to_a(self), instancepath_to_a(other));
}

void Init_instance_path(void)
{
	rb_define_alloc_func(cInstancePath, instancepath_alloc);
	rb_include_module(cInstancePath, rb_mEnumerable);
	rb_define_method(cInstancePath, "initialize", instancepath_initialize, 1);
	rb_define_method(cInstancePath, "length", instancepath_length, 0);
	rb_define_method(cInstancePath, "size", instancepath_length, 0);
	rb_define_method(cInstancePath, "to_a", instancepath_to_a, 0);
	rb_define_method(cInstancePath, "[]", instancepath_aref, 1);
	rb_define_method(cInstancePath, "each", instancepath_each, 0);
	rb_define_method(cInstancePath, "leaf", instancepath_leaf, 0);
	rb_define_method(cInstancePath, "root", instancepath_root, 0);
	rb_define_method(cInstancePath, "empty?", instancepath_empty_p, 0);
	rb_define_method(cInstancePath, "valid?", instancepath_valid_p, 0);
	rb_define_method(cInstancePath, "include?", instancepath_include_p, 1);
	rb_define_method(cInstancePath, "persistent_id_path", instancepath_persistent_id_path, 0);
	rb_define_method(cInstancePath, "transformation", instancepath_transformation, -1);
	rb_define_method(cInstancePath, "==", instancepath_eq, 1);
}
