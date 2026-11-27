#include <scriptup.h>

// Wraps the target entities. Geometry is added immediately, so it isn't merged.
#define VALID "__valid"

static VALUE builder_valid_p(VALUE self)
{
	return BOOL2RB(RTEST(rb_attr_get(self, (rb_intern)(VALID))));
}

static VALUE builder_entities_obj(VALUE self)
{
	if (!RTEST(builder_valid_p(self)))
		rb_raise(rb_eRuntimeError, "the builder is no longer valid");
	return su_entities_new(SU_REF(SUEntitiesRef, self));
}

static VALUE builder_entities(VALUE self)
{
	return builder_entities_obj(self);
}

static VALUE builder_add_edge(int argc, VALUE* argv, VALUE self)
{
	VALUE entities = builder_entities_obj(self);
	VALUE points = argc == 1 ? argv[0] : rb_ary_new_from_values(argc, argv);
	Check_Type(points, T_ARRAY);
	if (RARRAY_LEN(points) != 2)
		rb_raise(rb_eArgError, "expected 2 points");
	return rb_funcall(entities, (rb_intern)("add_line"), 2, RARRAY_AREF(points, 0), RARRAY_AREF(points, 1));
}

static VALUE builder_add_edges(int argc, VALUE* argv, VALUE self)
{
	return rb_funcallv(builder_entities_obj(self), (rb_intern)("add_edges"), argc, argv);
}

static void loop_points(VALUE points, struct SUPoint3D** out, long* count)
{
	Check_Type(points, T_ARRAY);
	*count = RARRAY_LEN(points);
	if (*count < 3)
		rb_raise(rb_eArgError, "a loop needs at least 3 points");
	*out = ALLOC_N(struct SUPoint3D, *count);
	for (long i = 0; i < *count; i++)
		(*out)[i] = su_to_point3d_like(RARRAY_AREF(points, i));
}

static VALUE builder_add_face(int argc, VALUE* argv, VALUE self)
{
	VALUE positional;
	VALUE options;
	rb_scan_args(argc, argv, "*:", &positional, &options);
	SUEntitiesRef entities = SU_REF(SUEntitiesRef, builder_entities_obj(self));
	VALUE outer = RARRAY_LEN(positional) == 1 && RB_TYPE_P(RARRAY_AREF(positional, 0), T_ARRAY) &&
		!rb_obj_is_kind_of(rb_ary_entry(RARRAY_AREF(positional, 0), 0), rb_cNumeric)
		? RARRAY_AREF(positional, 0) : positional;
	VALUE holes = Qnil;
	if (!NIL_P(options))
	{
		ID ids[1] = {(rb_intern)("holes")};
		VALUE values[1];
		rb_get_kwargs(options, ids, 0, 1, values);
		holes = values[0] == Qundef ? Qnil : values[0];
	}
	struct SUPoint3D* points;
	long count;
	loop_points(outer, &points, &count);
	SUFaceRef face = SU_INVALID;
	enum SUResult result = SUFaceCreateSimple(&face, points, (size_t)count);
	xfree(points);
	if (result != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "points are not planar or don't form a valid face");
	if (!NIL_P(holes))
	{
		Check_Type(holes, T_ARRAY);
		for (long h = 0; h < RARRAY_LEN(holes); h++)
		{
			struct SUPoint3D* hole;
			long hole_count;
			loop_points(RARRAY_AREF(holes, h), &hole, &hole_count);
			SULoopInputRef loop = SU_INVALID;
			SULoopInputCreate(&loop);
			for (long i = 0; i < hole_count; i++)
				SULoopInputAddVertexIndex(loop, (size_t)i);
			result = SUFaceAddInnerLoop(face, hole, &loop);
			xfree(hole);
			if (result != SU_ERROR_NONE)
				rb_raise(rb_eArgError, "invalid hole");
		}
	}
	SUEntitiesAddFaces(entities, 1, &face);
	return su_wrap(cFace, face.ptr);
}

static VALUE builder_vertex_at(VALUE self, VALUE position)
{
	struct SUPoint3D p = su_to_point3d(position);
	VALUE edges = rb_funcall(rb_funcall(builder_entities_obj(self), (rb_intern)("grep"), 1, cEdge), (rb_intern)("to_a"), 0);
	for (long i = 0; i < RARRAY_LEN(edges); i++)
	{
		VALUE vertices = rb_funcall(RARRAY_AREF(edges, i), (rb_intern)("vertices"), 0);
		for (long j = 0; j < RARRAY_LEN(vertices); j++)
		{
			struct SUPoint3D v = su_to_point3d_like(RARRAY_AREF(vertices, j));
			bool equal = false;
			SUPoint3DGetEqual(&v, &p, &equal);
			if (equal)
				return RARRAY_AREF(vertices, j);
		}
	}
	return Qnil;
}

static VALUE builder_invalidate(VALUE builder)
{
	rb_ivar_set(builder, (rb_intern)(VALID), Qfalse);
	return Qnil;
}

static VALUE builder_yield(VALUE builder)
{
	return rb_yield(builder);
}

// Entities#build: yields a builder that is only valid inside the block.
static VALUE entities_build(VALUE entities)
{
	VALUE builder = su_wrap(cEntitiesBuilder, DATA_PTR(entities));
	rb_ivar_set(builder, (rb_intern)(VALID), Qtrue);
	rb_ensure(builder_yield, builder, builder_invalidate, builder);
	return Qnil;
}

void Init_entities_builder(void)
{
	rb_undef_method(rb_singleton_class(cEntitiesBuilder), "new");
	rb_define_method(cEntitiesBuilder, "valid?", builder_valid_p, 0);
	rb_define_method(cEntitiesBuilder, "entities", builder_entities, 0);
	rb_define_method(cEntitiesBuilder, "add_edge", builder_add_edge, -1);
	rb_define_method(cEntitiesBuilder, "add_edges", builder_add_edges, -1);
	rb_define_method(cEntitiesBuilder, "add_face", builder_add_face, -1);
	rb_define_method(cEntitiesBuilder, "vertex_at", builder_vertex_at, 1);
	rb_define_method(cEntities, "build", entities_build, 0);
}
