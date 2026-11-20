#include <scriptup.h>

// Indices are 1-based. Negative polygon indices mark hidden edges.
struct polygon_mesh
{
	VALUE points;
	VALUE polygons;
	VALUE uvs_front;
	VALUE uvs_back;
	VALUE normals;
};

#define MESH(obj) ((struct polygon_mesh*)DATA_PTR(obj))

static void mesh_mark(void* ptr)
{
	struct polygon_mesh* m = ptr;
	rb_gc_mark(m->points);
	rb_gc_mark(m->polygons);
	rb_gc_mark(m->uvs_front);
	rb_gc_mark(m->uvs_back);
	rb_gc_mark(m->normals);
}

static VALUE polygonmesh_alloc(VALUE klass)
{
	struct polygon_mesh* m;
	VALUE obj = Data_Make_Struct(klass, struct polygon_mesh, mesh_mark, RUBY_DEFAULT_FREE, m);
	m->points = rb_ary_new();
	m->polygons = rb_ary_new();
	m->uvs_front = rb_ary_new();
	m->uvs_back = rb_ary_new();
	m->normals = rb_ary_new();
	return obj;
}

// Builds a mesh from raw data (used by Face#mesh).
VALUE su_polygon_mesh_new(VALUE points, VALUE polygons, VALUE uvs_front, VALUE uvs_back, VALUE normals)
{
	VALUE obj = polygonmesh_alloc(cPolygonMesh);
	struct polygon_mesh* m = MESH(obj);
	m->points = points;
	m->polygons = polygons;
	m->uvs_front = uvs_front;
	m->uvs_back = uvs_back;
	m->normals = normals;
	return obj;
}

static VALUE mesh_entry(VALUE ary, VALUE index)
{
	long i = NUM2LONG(index);
	if (i < 1 || i > RARRAY_LEN(ary))
		return Qnil;
	return RARRAY_AREF(ary, i - 1);
}

static VALUE polygonmesh_initialize(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 0, 2);
	for (int i = 0; i < argc; i++)
		NUM2LONG(argv[i]); // capacity hints only
	return self;
}

static VALUE polygonmesh_point_index(VALUE self, VALUE point)
{
	VALUE p = su_point3d_new(su_to_point3d(point));
	VALUE points = MESH(self)->points;
	for (long i = 0; i < RARRAY_LEN(points); i++)
		if (rb_equal(RARRAY_AREF(points, i), p))
			return LONG2NUM(i + 1);
	return INT2FIX(0);
}

static VALUE polygonmesh_add_point(VALUE self, VALUE point)
{
	VALUE index = polygonmesh_point_index(self, point);
	if (index != INT2FIX(0))
		return index;
	rb_ary_push(MESH(self)->points, su_point3d_new(su_to_point3d(point)));
	return LONG2NUM(RARRAY_LEN(MESH(self)->points));
}

static VALUE polygonmesh_add_polygon(int argc, VALUE* argv, VALUE self)
{
	VALUE items = argc == 1 && RB_TYPE_P(argv[0], T_ARRAY) ? argv[0] : rb_ary_new_from_values(argc, argv);
	if (RARRAY_LEN(items) < 3)
		rb_raise(rb_eArgError, "a polygon needs at least 3 points");
	VALUE polygon = rb_ary_new_capa(RARRAY_LEN(items));
	long count = RARRAY_LEN(MESH(self)->points);
	for (long i = 0; i < RARRAY_LEN(items); i++)
	{
		VALUE item = RARRAY_AREF(items, i);
		if (RB_INTEGER_TYPE_P(item))
		{
			long index = NUM2LONG(item);
			if (index == 0 || labs(index) > count)
				rb_raise(rb_eArgError, "invalid point index %ld", index);
			rb_ary_push(polygon, item);
		}
		else
			rb_ary_push(polygon, polygonmesh_add_point(self, item));
	}
	rb_ary_push(MESH(self)->polygons, polygon);
	return LONG2NUM(RARRAY_LEN(MESH(self)->polygons));
}

static VALUE polygonmesh_count_points(VALUE self)
{
	return LONG2NUM(RARRAY_LEN(MESH(self)->points));
}

static VALUE polygonmesh_count_polygons(VALUE self)
{
	return LONG2NUM(RARRAY_LEN(MESH(self)->polygons));
}

static VALUE polygonmesh_point_at(VALUE self, VALUE index)
{
	VALUE point = mesh_entry(MESH(self)->points, index);
	return NIL_P(point) ? Qnil : su_point3d_new(su_to_point3d(point));
}

static VALUE polygonmesh_points(VALUE self)
{
	VALUE points = MESH(self)->points;
	VALUE result = rb_ary_new_capa(RARRAY_LEN(points));
	for (long i = 0; i < RARRAY_LEN(points); i++)
		rb_ary_push(result, su_point3d_new(su_to_point3d(RARRAY_AREF(points, i))));
	return result;
}

static VALUE polygonmesh_polygon_at(VALUE self, VALUE index)
{
	VALUE polygon = mesh_entry(MESH(self)->polygons, index);
	return NIL_P(polygon) ? Qnil : rb_ary_dup(polygon);
}

static VALUE polygonmesh_polygon_points_at(VALUE self, VALUE index)
{
	VALUE polygon = mesh_entry(MESH(self)->polygons, index);
	if (NIL_P(polygon))
		return Qnil;
	VALUE result = rb_ary_new_capa(RARRAY_LEN(polygon));
	for (long i = 0; i < RARRAY_LEN(polygon); i++)
		rb_ary_push(result, polygonmesh_point_at(self, LONG2NUM(labs(NUM2LONG(RARRAY_AREF(polygon, i))))));
	return result;
}

static VALUE polygonmesh_polygons(VALUE self)
{
	VALUE polygons = MESH(self)->polygons;
	VALUE result = rb_ary_new_capa(RARRAY_LEN(polygons));
	for (long i = 0; i < RARRAY_LEN(polygons); i++)
		rb_ary_push(result, rb_ary_dup(RARRAY_AREF(polygons, i)));
	return result;
}

static VALUE polygonmesh_set_point(VALUE self, VALUE index, VALUE point)
{
	long i = NUM2LONG(index);
	if (i < 1 || i > RARRAY_LEN(MESH(self)->points))
		rb_raise(rb_eIndexError, "invalid point index %ld", i);
	rb_ary_store(MESH(self)->points, i - 1, su_point3d_new(su_to_point3d(point)));
	return self;
}

static VALUE uv_array(VALUE self, VALUE front)
{
	return RTEST(front) ? MESH(self)->uvs_front : MESH(self)->uvs_back;
}

static VALUE polygonmesh_set_uv(VALUE self, VALUE index, VALUE point, VALUE front)
{
	long i = NUM2LONG(index);
	if (i < 1 || i > RARRAY_LEN(MESH(self)->points))
		rb_raise(rb_eIndexError, "invalid point index %ld", i);
	rb_ary_store(uv_array(self, front), i - 1, su_point3d_new(su_to_point3d(point)));
	return Qnil;
}

static VALUE polygonmesh_uv_at(VALUE self, VALUE index, VALUE front)
{
	VALUE uv = mesh_entry(uv_array(self, front), index);
	return NIL_P(uv) ? Qnil : su_point3d_new(su_to_point3d(uv));
}

static VALUE polygonmesh_uvs(VALUE self, VALUE front)
{
	VALUE uvs = uv_array(self, front);
	VALUE result = rb_ary_new_capa(RARRAY_LEN(uvs));
	for (long i = 0; i < RARRAY_LEN(uvs); i++)
	{
		VALUE uv = RARRAY_AREF(uvs, i);
		rb_ary_push(result, NIL_P(uv) ? Qnil : su_point3d_new(su_to_point3d(uv)));
	}
	return result;
}

static VALUE polygonmesh_normal_at(VALUE self, VALUE index)
{
	VALUE normal = mesh_entry(MESH(self)->normals, index);
	return NIL_P(normal) ? Qnil : su_vector3d_new(su_to_vector3d(normal));
}

static VALUE polygonmesh_transform_bang(VALUE self, VALUE transformation)
{
	struct SUTransformation t = su_to_transformation(transformation);
	VALUE points = MESH(self)->points;
	for (long i = 0; i < RARRAY_LEN(points); i++)
	{
		struct SUPoint3D p = su_to_point3d(RARRAY_AREF(points, i));
		SUPoint3DTransform(&t, &p);
		rb_ary_store(points, i, su_point3d_new(p));
	}
	VALUE normals = MESH(self)->normals;
	for (long i = 0; i < RARRAY_LEN(normals); i++)
	{
		struct SUVector3D v = su_to_vector3d(RARRAY_AREF(normals, i));
		SUVector3DTransform(&t, &v);
		rb_ary_store(normals, i, su_vector3d_new(v));
	}
	return self;
}

void Init_polygon_mesh(void)
{
	static const struct { const char* name; int value; } constants[] = {
		{"NO_SMOOTH_OR_HIDE", 0}, {"HIDE_BASED_ON_INDEX", 1}, {"SOFTEN_BASED_ON_INDEX", 2}, {"AUTO_SOFTEN", 4},
		{"SMOOTH_SOFT_EDGES", 8}, {"MESH_POINTS", 0}, {"MESH_UVQ_FRONT", 1}, {"MESH_UVQ_BACK", 3}, {"MESH_NORMALS", 4},
	};
	for (size_t i = 0; i < sizeof(constants) / sizeof(constants[0]); i++)
		rb_define_const(cPolygonMesh, constants[i].name, INT2FIX(constants[i].value));
	rb_define_alloc_func(cPolygonMesh, polygonmesh_alloc);
	rb_define_method(cPolygonMesh, "initialize", polygonmesh_initialize, -1);
	rb_define_method(cPolygonMesh, "add_point", polygonmesh_add_point, 1);
	rb_define_method(cPolygonMesh, "add_polygon", polygonmesh_add_polygon, -1);
	rb_define_method(cPolygonMesh, "count_points", polygonmesh_count_points, 0);
	rb_define_method(cPolygonMesh, "count_polygons", polygonmesh_count_polygons, 0);
	rb_define_method(cPolygonMesh, "point_at", polygonmesh_point_at, 1);
	rb_define_method(cPolygonMesh, "point_index", polygonmesh_point_index, 1);
	rb_define_method(cPolygonMesh, "points", polygonmesh_points, 0);
	rb_define_method(cPolygonMesh, "polygon_at", polygonmesh_polygon_at, 1);
	rb_define_method(cPolygonMesh, "polygon_points_at", polygonmesh_polygon_points_at, 1);
	rb_define_method(cPolygonMesh, "polygons", polygonmesh_polygons, 0);
	rb_define_method(cPolygonMesh, "set_point", polygonmesh_set_point, 2);
	rb_define_method(cPolygonMesh, "set_uv", polygonmesh_set_uv, 3);
	rb_define_method(cPolygonMesh, "uv_at", polygonmesh_uv_at, 2);
	rb_define_method(cPolygonMesh, "uvs", polygonmesh_uvs, 1);
	rb_define_method(cPolygonMesh, "normal_at", polygonmesh_normal_at, 1);
	rb_define_method(cPolygonMesh, "transform!", polygonmesh_transform_bang, 1);
}
