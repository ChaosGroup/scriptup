#include <scriptup.h>

#define FACE(self) SU_REF(SUFaceRef, self)
#define FACE_TOLERANCE 1.0e-3

enum
{
	POINT_UNKNOWN = 0,
	POINT_INSIDE = 1,
	POINT_ON_VERTEX = 2,
	POINT_ON_EDGE = 4,
	POINT_ON_FACE = 8,
	POINT_OUTSIDE = 16,
	POINT_NOT_ON_PLANE = 32
};

DEFINE_REF_LIST(face_edge_list, SUFaceRef, SUFaceGetNumEdges, SUFaceGetEdges, SUEdgeRef, cEdge)
DEFINE_REF_LIST(face_vertex_list, SUFaceRef, SUFaceGetNumVertices, SUFaceGetVertices, SUVertexRef, cVertex)
DEFINE_REF_LIST(face_inner_loop_list, SUFaceRef, SUFaceGetNumInnerLoops, SUFaceGetInnerLoops, SULoopRef, cLoop)
DEFINE_REF_LIST(loop_vertex_list, SULoopRef, SULoopGetNumVertices, SULoopGetVertices, SUVertexRef, cVertex)
DEFINE_REF_GETTER(face_outer_loop, SUFaceRef, SUFaceGetOuterLoop, SULoopRef, cLoop)
DEFINE_REF_GETTER(face_material, SUFaceRef, SUFaceGetFrontMaterial, SUMaterialRef, cMaterial)
DEFINE_REF_GETTER(face_back_material, SUFaceRef, SUFaceGetBackMaterial, SUMaterialRef, cMaterial)

static VALUE face_edges(VALUE self) { return face_edge_list(FACE(self)); }
static VALUE face_vertices(VALUE self) { return face_vertex_list(FACE(self)); }

static VALUE face_loops(VALUE self)
{
	VALUE loops = rb_ary_new_from_args(1, face_outer_loop(self));
	return rb_ary_concat(loops, face_inner_loop_list(FACE(self)));
}

static VALUE face_normal(VALUE self)
{
	struct SUVector3D normal = {0, 0, 0};
	SUFaceGetNormal(FACE(self), &normal);
	return su_vector3d_new(normal);
}

static VALUE face_plane(VALUE self)
{
	struct SUPlane3D plane = {0, 0, 0, 0};
	SUFaceGetPlane(FACE(self), &plane);
	return rb_ary_new_from_args(4, DBL2NUM(plane.a), DBL2NUM(plane.b), DBL2NUM(plane.c), DBL2NUM(plane.d));
}

static VALUE face_area(int argc, VALUE* argv, VALUE self)
{
	VALUE transformation;
	rb_scan_args(argc, argv, "01", &transformation);
	double area = 0.0;
	if (NIL_P(transformation))
		SUFaceGetArea(FACE(self), &area);
	else
	{
		struct SUTransformation t = su_to_transformation(transformation);
		SUFaceGetAreaWithTransform(FACE(self), &t, &area);
	}
	return DBL2NUM(area);
}

static VALUE face_set_material(VALUE self, VALUE material)
{
	SUFaceSetFrontMaterial(FACE(self), su_material_from_arg(self, material));
	return material;
}

static VALUE face_set_back_material(VALUE self, VALUE material)
{
	SUFaceSetBackMaterial(FACE(self), su_material_from_arg(self, material));
	return material;
}

static VALUE face_reverse_bang(VALUE self)
{
	return SUFaceReverse(FACE(self)) == SU_ERROR_NONE ? self : Qfalse;
}

static VALUE face_coplanar_with_p(VALUE self, VALUE other)
{
	bool coplanar = false;
	SUFaceCoplanarWith(FACE(self), (SUFaceRef){su_unwrap(other, cFace)}, &coplanar);
	return BOOL2RB(coplanar);
}

static VALUE face_all_connected(VALUE self)
{
	VALUE edges = face_edges(self);
	if (RARRAY_LEN(edges) == 0)
		return rb_ary_new_from_args(1, self);
	return rb_funcall(RARRAY_AREF(edges, 0), (rb_intern)("all_connected"), 0);
}

static VALUE face_get_glued_instances(VALUE self)
{
	size_t count = 0;
	SUFaceGetNumAttachedDrawingElements(FACE(self), &count);
	VALUE ary = rb_ary_new_capa((long)count);
	if (count == 0)
		return ary;
	SUDrawingElementRef* elements = ZALLOC_N(SUDrawingElementRef, count);
	SUFaceGetAttachedDrawingElements(FACE(self), count, elements, &count);
	for (size_t i = 0; i < count; i++)
		rb_ary_push(ary, su_wrap_entity((SUEntityRef){elements[i].ptr}));
	xfree(elements);
	return ary;
}

#define DEFINE_SIDE_PREDICATE(fn, SUFn) \
	static VALUE fn(VALUE self, VALUE front) \
	{ \
		bool result = false; \
		SUFn(FACE(self), RTEST(front), &result); \
		return BOOL2RB(result); \
	}
DEFINE_SIDE_PREDICATE(face_texture_positioned_p, SUFaceIsTexturePositioned)
DEFINE_SIDE_PREDICATE(face_texture_projected_p, SUFaceIsTextureProjected)

static VALUE face_clear_texture_position(VALUE self, VALUE front)
{
	return BOOL2RB(SUFaceClearTexturePosition(FACE(self), RTEST(front)) == SU_ERROR_NONE);
}

static VALUE face_clear_texture_projection(VALUE self, VALUE front)
{
	return BOOL2RB(SUFaceClearTextureProjection(FACE(self), RTEST(front)) == SU_ERROR_NONE);
}

static VALUE face_get_texture_projection(VALUE self, VALUE front)
{
	struct SUVector3D vector = {0, 0, 0};
	if (SUFaceGetTextureProjection(FACE(self), RTEST(front), &vector) != SU_ERROR_NONE)
		return Qnil;
	return su_vector3d_new(vector);
}

static VALUE face_uv_tile_at(VALUE self, VALUE position, VALUE front)
{
	struct SUPoint3D reference = su_to_point3d(position);
	struct SUPoint3D points[4];
	struct SUPoint2D uvs[4];
	if (SUFaceGetUVTileAt(FACE(self), &reference, RTEST(front), points, uvs) != SU_ERROR_NONE)
		return Qnil;
	VALUE result = rb_ary_new_capa(8);
	for (int i = 0; i < 4; i++)
	{
		rb_ary_push(result, su_point3d_new(points[i]));
		rb_ary_push(result, su_point3d_new((struct SUPoint3D){uvs[i].x, uvs[i].y, 1.0}));
	}
	return result;
}

// mapping: [model point, uv point, ...] with 1 to 4 pairs.
static VALUE face_position_material(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 3, 4);
	Check_Type(argv[1], T_ARRAY);
	long length = RARRAY_LEN(argv[1]);
	if (length < 2 || length > 8 || length % 2 != 0)
		rb_raise(rb_eArgError, "expected 1 to 4 pairs of model and UV points");
	struct SUMaterialPositionInput mapping;
	memset(&mapping, 0, sizeof(mapping));
	mapping.material.ptr = su_unwrap(argv[0], cMaterial);
	mapping.num_uv_coords = (size_t)length / 2;
	for (long i = 0; i < length / 2; i++)
	{
		mapping.points[i] = su_to_point3d(RARRAY_AREF(argv[1], i * 2));
		struct SUPoint3D uv = su_to_point3d(RARRAY_AREF(argv[1], i * 2 + 1));
		mapping.uv_coords[i] = (struct SUPoint2D){uv.x, uv.y};
	}
	if (argc == 4 && !NIL_P(argv[3]))
		mapping.projection = su_to_vector3d(argv[3]);
	if (SUFacePositionMaterial(FACE(self), RTEST(argv[2]), &mapping) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "cannot position material");
	return self;
}

static VALUE face_mesh(int argc, VALUE* argv, VALUE self)
{
	VALUE flags;
	rb_scan_args(argc, argv, "01", &flags);
	SUMeshHelperRef helper = SU_INVALID;
	if (SUMeshHelperCreate(&helper, FACE(self)) != SU_ERROR_NONE)
		rb_raise(rb_eRuntimeError, "cannot triangulate face");
	size_t vertex_count = 0;
	size_t triangle_count = 0;
	SUMeshHelperGetNumVertices(helper, &vertex_count);
	SUMeshHelperGetNumTriangles(helper, &triangle_count);
	struct SUPoint3D* vertices = ZALLOC_N(struct SUPoint3D, vertex_count + 1);
	struct SUPoint3D* front = ZALLOC_N(struct SUPoint3D, vertex_count + 1);
	struct SUPoint3D* back = ZALLOC_N(struct SUPoint3D, vertex_count + 1);
	struct SUVector3D* normals = ZALLOC_N(struct SUVector3D, vertex_count + 1);
	size_t* indices = ZALLOC_N(size_t, triangle_count * 3 + 1);
	size_t n = 0;
	SUMeshHelperGetVertices(helper, vertex_count, vertices, &n);
	SUMeshHelperGetFrontSTQCoords(helper, vertex_count, front, &n);
	SUMeshHelperGetBackSTQCoords(helper, vertex_count, back, &n);
	SUMeshHelperGetNormals(helper, vertex_count, normals, &n);
	SUMeshHelperGetVertexIndices(helper, triangle_count * 3, indices, &n);
	SUMeshHelperRelease(&helper);

	VALUE points = rb_ary_new_capa((long)vertex_count);
	VALUE uvs_front = rb_ary_new_capa((long)vertex_count);
	VALUE uvs_back = rb_ary_new_capa((long)vertex_count);
	VALUE normal_list = rb_ary_new_capa((long)vertex_count);
	for (size_t i = 0; i < vertex_count; i++)
	{
		rb_ary_push(points, su_point3d_new(vertices[i]));
		rb_ary_push(uvs_front, su_point3d_new(front[i]));
		rb_ary_push(uvs_back, su_point3d_new(back[i]));
		rb_ary_push(normal_list, su_vector3d_new(normals[i]));
	}
	VALUE polygons = rb_ary_new_capa((long)triangle_count);
	for (size_t i = 0; i < triangle_count; i++)
		rb_ary_push(polygons, rb_ary_new_from_args(3, SIZET2NUM(indices[i * 3] + 1), SIZET2NUM(indices[i * 3 + 1] + 1),
			SIZET2NUM(indices[i * 3 + 2] + 1)));
	xfree(vertices);
	xfree(front);
	xfree(back);
	xfree(normals);
	xfree(indices);
	return su_polygon_mesh_new(points, polygons, uvs_front, uvs_back, normal_list);
}

// Projects onto the plane's dominant axes for 2D tests.
static void project(struct SUPoint3D p, int axis, double* u, double* v)
{
	double c[3] = {p.x, p.y, p.z};
	*u = c[(axis + 1) % 3];
	*v = c[(axis + 2) % 3];
}

static bool in_loop(VALUE vertices, struct SUPoint3D p, int axis)
{
	double pu;
	double pv;
	project(p, axis, &pu, &pv);
	bool inside = false;
	long count = RARRAY_LEN(vertices);
	for (long i = 0, j = count - 1; i < count; j = i++)
	{
		double au, av, bu, bv;
		project(su_to_point3d_like(RARRAY_AREF(vertices, i)), axis, &au, &av);
		project(su_to_point3d_like(RARRAY_AREF(vertices, j)), axis, &bu, &bv);
		if ((av > pv) != (bv > pv) && pu < (bu - au) * (pv - av) / (bv - av) + au)
			inside = !inside;
	}
	return inside;
}

static VALUE face_classify_point(VALUE self, VALUE point)
{
	struct SUPoint3D p = su_to_point3d(point);
	struct SUPlane3D plane;
	SUFaceGetPlane(FACE(self), &plane);
	double distance = 0.0;
	SUPlane3DDistanceTo(&plane, &p, &distance);
	if (fabs(distance) > FACE_TOLERANCE)
		return INT2FIX(POINT_NOT_ON_PLANE);
	VALUE vertices = face_vertices(self);
	for (long i = 0; i < RARRAY_LEN(vertices); i++)
	{
		struct SUPoint3D v = su_to_point3d_like(RARRAY_AREF(vertices, i));
		double d = 0.0;
		SUPoint3DDistanceToSUPoint3D(&v, &p, &d);
		if (d < FACE_TOLERANCE)
			return INT2FIX(POINT_ON_VERTEX);
	}
	VALUE edges = face_edges(self);
	for (long i = 0; i < RARRAY_LEN(edges); i++)
	{
		VALUE edge = RARRAY_AREF(edges, i);
		struct SUPoint3D a = su_to_point3d_like(rb_funcall(edge, (rb_intern)("start"), 0));
		struct SUPoint3D b = su_to_point3d_like(rb_funcall(edge, (rb_intern)("end"), 0));
		struct SURay3D ray = {a, {b.x - a.x, b.y - a.y, b.z - a.z}};
		double length = sqrt(ray.normal.x * ray.normal.x + ray.normal.y * ray.normal.y + ray.normal.z * ray.normal.z);
		SUVector3DNormalize(&ray.normal);
		struct SUPoint3D projected;
		SURay3DProjectTo(&ray, &p, &projected);
		double d = 0.0;
		SUPoint3DDistanceToSUPoint3D(&projected, &p, &d);
		double t = (projected.x - a.x) * ray.normal.x + (projected.y - a.y) * ray.normal.y + (projected.z - a.z) * ray.normal.z;
		if (d < FACE_TOLERANCE && t >= -FACE_TOLERANCE && t <= length + FACE_TOLERANCE)
			return INT2FIX(POINT_ON_EDGE);
	}
	double normal[3] = {fabs(plane.a), fabs(plane.b), fabs(plane.c)};
	int axis = normal[0] >= normal[1] && normal[0] >= normal[2] ? 0 : normal[1] >= normal[2] ? 1 : 2;
	VALUE loops = face_loops(self);
	if (!in_loop(loop_vertex_list(SU_REF(SULoopRef, RARRAY_AREF(loops, 0))), p, axis))
		return INT2FIX(POINT_OUTSIDE);
	for (long i = 1; i < RARRAY_LEN(loops); i++)
		if (in_loop(loop_vertex_list(SU_REF(SULoopRef, RARRAY_AREF(loops, i))), p, axis))
			return INT2FIX(POINT_OUTSIDE);
	return INT2FIX(POINT_INSIDE);
}

void Init_face(void)
{
	rb_define_const(cFace, "PointUnknown", INT2FIX(POINT_UNKNOWN));
	rb_define_const(cFace, "PointInside", INT2FIX(POINT_INSIDE));
	rb_define_const(cFace, "PointOnVertex", INT2FIX(POINT_ON_VERTEX));
	rb_define_const(cFace, "PointOnEdge", INT2FIX(POINT_ON_EDGE));
	rb_define_const(cFace, "PointOnFace", INT2FIX(POINT_ON_FACE));
	rb_define_const(cFace, "PointOutside", INT2FIX(POINT_OUTSIDE));
	rb_define_const(cFace, "PointNotOnPlane", INT2FIX(POINT_NOT_ON_PLANE));
	rb_define_method(cFace, "edges", face_edges, 0);
	rb_define_method(cFace, "vertices", face_vertices, 0);
	rb_define_method(cFace, "loops", face_loops, 0);
	rb_define_method(cFace, "outer_loop", face_outer_loop, 0);
	rb_define_method(cFace, "normal", face_normal, 0);
	rb_define_method(cFace, "plane", face_plane, 0);
	rb_define_method(cFace, "area", face_area, -1);
	rb_define_method(cFace, "material", face_material, 0);
	rb_define_method(cFace, "material=", face_set_material, 1);
	rb_define_method(cFace, "back_material", face_back_material, 0);
	rb_define_method(cFace, "back_material=", face_set_back_material, 1);
	rb_define_method(cFace, "reverse!", face_reverse_bang, 0);
	rb_define_method(cFace, "coplanar_with?", face_coplanar_with_p, 1);
	rb_define_method(cFace, "all_connected", face_all_connected, 0);
	rb_define_method(cFace, "get_glued_instances", face_get_glued_instances, 0);
	rb_define_method(cFace, "texture_positioned?", face_texture_positioned_p, 1);
	rb_define_method(cFace, "texture_projected?", face_texture_projected_p, 1);
	rb_define_method(cFace, "clear_texture_position", face_clear_texture_position, 1);
	rb_define_method(cFace, "clear_texture_projection", face_clear_texture_projection, 1);
	rb_define_method(cFace, "get_texture_projection", face_get_texture_projection, 1);
	rb_define_method(cFace, "uv_tile_at", face_uv_tile_at, 2);
	rb_define_method(cFace, "position_material", face_position_material, -1);
	rb_define_method(cFace, "mesh", face_mesh, -1);
	rb_define_method(cFace, "classify_point", face_classify_point, 1);
}
