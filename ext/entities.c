#include <scriptup.h>

#define ENTITIES(self) SU_REF(SUEntitiesRef, self)

DEFINE_REF_LIST(entities_faces, SUEntitiesRef, SUEntitiesGetNumFaces, SUEntitiesGetFaces, SUFaceRef, cFace)
DEFINE_REF_LIST(entities_guide_points, SUEntitiesRef, SUEntitiesGetNumGuidePoints, SUEntitiesGetGuidePoints, SUGuidePointRef, cConstructionPoint)
DEFINE_REF_LIST(entities_guide_lines, SUEntitiesRef, SUEntitiesGetNumGuideLines, SUEntitiesGetGuideLines, SUGuideLineRef, cConstructionLine)
DEFINE_REF_LIST(entities_groups, SUEntitiesRef, SUEntitiesGetNumGroups, SUEntitiesGetGroups, SUGroupRef, cGroup)
DEFINE_REF_LIST(entities_images, SUEntitiesRef, SUEntitiesGetNumImages, SUEntitiesGetImages, SUImageRef, cImage)
DEFINE_REF_LIST(entities_instances, SUEntitiesRef, SUEntitiesGetNumInstances, SUEntitiesGetInstances, SUComponentInstanceRef, cComponentInstance)
DEFINE_REF_LIST(entities_section_planes, SUEntitiesRef, SUEntitiesGetNumSectionPlanes, SUEntitiesGetSectionPlanes, SUSectionPlaneRef, cSectionPlane)
DEFINE_REF_LIST(entities_texts, SUEntitiesRef, SUEntitiesGetNumTexts, SUEntitiesGetTexts, SUTextRef, cText)
DEFINE_REF_LIST(entities_dimensions, SUEntitiesRef, SUEntitiesGetNumDimensions, SUEntitiesGetDimensions, SUDimensionRef, cDimension)
DEFINE_REF_LIST(curve_edges, SUCurveRef, SUCurveGetNumEdges, SUCurveGetEdges, SUEdgeRef, cEdge)

VALUE su_entities_new(SUEntitiesRef entities)
{
	return su_wrap_or_nil(cEntities, entities.ptr);
}

static VALUE entities_edges(SUEntitiesRef entities)
{
	size_t count = 0;
	SUEntitiesGetNumEdges(entities, false, &count);
	VALUE ary = rb_ary_new_capa((long)count);
	if (count == 0)
		return ary;
	SUEdgeRef* edges = ALLOC_N(SUEdgeRef, count);
	SUEntitiesGetEdges(entities, false, count, edges, &count);
	for (size_t i = 0; i < count; i++)
		rb_ary_push(ary, su_wrap(cEdge, edges[i].ptr));
	xfree(edges);
	return ary;
}

// Every entity, wrapped as its runtime class.
static VALUE entities_to_a(VALUE self)
{
	SUEntitiesRef entities = ENTITIES(self);
	VALUE (*lists[])(SUEntitiesRef) = {entities_edges, entities_faces, entities_guide_points, entities_guide_lines,
		entities_groups, entities_images, entities_instances, entities_section_planes, entities_texts, entities_dimensions};
	VALUE all = rb_ary_new();
	for (size_t i = 0; i < sizeof(lists) / sizeof(lists[0]); i++)
		rb_ary_concat(all, lists[i](entities));
	for (long i = 0; i < RARRAY_LEN(all); i++)
		rb_ary_store(all, i, su_wrap_entity((SUEntityRef){DATA_PTR(RARRAY_AREF(all, i))}));
	return all;
}

static VALUE entities_each(VALUE self)
{
	su_yield_each(self, entities_to_a(self));
	return Qnil;
}

static VALUE entities_aref(VALUE self, VALUE index)
{
	return rb_ary_entry(entities_to_a(self), NUM2LONG(index));
}

static VALUE entities_length(VALUE self)
{
	return LONG2NUM(RARRAY_LEN(entities_to_a(self)));
}

static VALUE entities_model(VALUE self)
{
	struct SUEntitiesParent parent = {SU_INVALID, SU_INVALID};
	SUEntitiesGetParent(ENTITIES(self), &parent);
	if (SUIsValid(parent.model))
		return su_wrap(cModel, parent.model.ptr);
	SUModelRef model = SU_INVALID;
	SUEntityGetModel((SUEntityRef){parent.definition.ptr}, &model);
	return su_wrap_or_nil(cModel, model.ptr);
}

static VALUE entities_parent(VALUE self)
{
	struct SUEntitiesParent parent = {SU_INVALID, SU_INVALID};
	SUEntitiesGetParent(ENTITIES(self), &parent);
	if (SUIsValid(parent.definition))
		return su_wrap(cComponentDefinition, parent.definition.ptr);
	return su_wrap_or_nil(cModel, parent.model.ptr);
}

static VALUE entities_eq(VALUE self, VALUE other)
{
	return BOOL2RB(rb_obj_is_kind_of(other, cEntities) && DATA_PTR(self) == DATA_PTR(other));
}

// Flattens (*args) or ([array]) into one array.
static VALUE varargs(int argc, VALUE* argv)
{
	if (argc == 1 && RB_TYPE_P(argv[0], T_ARRAY))
	{
		VALUE first = RARRAY_LEN(argv[0]) > 0 ? RARRAY_AREF(argv[0], 0) : Qnil;
		if (!rb_obj_is_kind_of(first, rb_cNumeric))
			return argv[0];
	}
	return rb_ary_new_from_values(argc, argv);
}

static VALUE add_edge(SUEntitiesRef entities, struct SUPoint3D a, struct SUPoint3D b)
{
	bool equal = false;
	SUPoint3DGetEqual(&a, &b, &equal);
	if (equal)
		return Qnil;
	SUEdgeRef edge = SU_INVALID;
	SUEdgeCreate(&edge, &a, &b);
	SUEntitiesAddEdges(entities, 1, &edge);
	return su_wrap(cEdge, edge.ptr);
}

static VALUE entities_add_line(VALUE self, VALUE p1, VALUE p2)
{
	return add_edge(ENTITIES(self), su_to_point3d_like(p1), su_to_point3d_like(p2));
}

static VALUE entities_add_edges(int argc, VALUE* argv, VALUE self)
{
	VALUE points = varargs(argc, argv);
	if (RARRAY_LEN(points) < 2)
		rb_raise(rb_eArgError, "at least 2 points are required");
	VALUE edges = rb_ary_new();
	for (long i = 0; i + 1 < RARRAY_LEN(points); i++)
	{
		VALUE edge = add_edge(ENTITIES(self), su_to_point3d_like(RARRAY_AREF(points, i)), su_to_point3d_like(RARRAY_AREF(points, i + 1)));
		if (!NIL_P(edge))
			rb_ary_push(edges, edge);
	}
	return edges;
}

static struct SUPoint3D edge_start(VALUE edge)
{
	SUVertexRef vertex = SU_INVALID;
	SUEdgeGetStartVertex(SU_REF(SUEdgeRef, edge), &vertex);
	struct SUPoint3D p = {0, 0, 0};
	SUVertexGetPosition(vertex, &p);
	return p;
}

static VALUE entities_add_face(int argc, VALUE* argv, VALUE self)
{
	VALUE items = varargs(argc, argv);
	if (RARRAY_LEN(items) == 1 && rb_obj_is_kind_of(RARRAY_AREF(items, 0), cCurve))
		items = curve_edges((SUCurveRef){DATA_PTR(RARRAY_AREF(items, 0))});
	long count = RARRAY_LEN(items);
	if (count < 3)
		rb_raise(rb_eArgError, "at least 3 points or edges are required");
	struct SUPoint3D* points = ALLOC_N(struct SUPoint3D, count);
	for (long i = 0; i < count; i++)
	{
		VALUE item = RARRAY_AREF(items, i);
		points[i] = rb_obj_is_kind_of(item, cEdge) ? edge_start(item) : su_to_point3d_like(item);
	}
	SUFaceRef face = SU_INVALID;
	enum SUResult result = SUFaceCreateSimple(&face, points, (size_t)count);
	xfree(points);
	if (result != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "points are not planar or don't form a valid face");
	SUEntitiesAddFaces(ENTITIES(self), 1, &face);
	return su_wrap(cFace, face.ptr);
}

static VALUE add_arc_curve(VALUE self, struct SUPoint3D center, struct SUVector3D xaxis, struct SUVector3D normal,
	double radius, double start_angle, double end_angle, long segments)
{
	if (radius <= 0)
		rb_raise(rb_eArgError, "radius must be positive");
	if (segments < 1)
		rb_raise(rb_eArgError, "number of segments must be positive");
	if (SUVector3DNormalize(&xaxis) != SU_ERROR_NONE || SUVector3DNormalize(&normal) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "invalid axis");
	struct SUVector3D yaxis;
	SUVector3DCross(&normal, &xaxis, &yaxis);
	struct SUPoint3D ends[2];
	double angles[2] = {start_angle, end_angle};
	for (int i = 0; i < 2; i++)
	{
		double c = cos(angles[i]) * radius;
		double s = sin(angles[i]) * radius;
		ends[i] = (struct SUPoint3D){center.x + xaxis.x * c + yaxis.x * s, center.y + xaxis.y * c + yaxis.y * s,
			center.z + xaxis.z * c + yaxis.z * s};
	}
	SUArcCurveRef arc = SU_INVALID;
	if (SUArcCurveCreate(&arc, &center, &ends[0], &ends[1], &normal, (size_t)segments) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "cannot create arc");
	SUEntitiesAddArcCurves(ENTITIES(self), 1, &arc);
	return curve_edges(SUArcCurveToCurve(arc));
}

static VALUE entities_add_arc(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 6, 7);
	double start_angle = NUM2DBL(argv[4]);
	double end_angle = NUM2DBL(argv[5]);
	long segments = argc == 7 ? NUM2LONG(argv[6]) : (long)ceil(fabs(end_angle - start_angle) / (2 * M_PI) * 24);
	return add_arc_curve(self, su_to_point3d(argv[0]), su_to_vector3d(argv[1]), su_to_vector3d(argv[2]),
		NUM2DBL(argv[3]), start_angle, end_angle, segments < 1 ? 1 : segments);
}

static VALUE entities_add_circle(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 3, 4);
	struct SUVector3D normal = su_to_vector3d(argv[1]);
	struct SUVector3D xaxis;
	struct SUVector3D yaxis;
	if (SUVector3DGetArbitraryAxes(&normal, &xaxis, &yaxis) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "invalid normal");
	long segments = argc == 4 ? NUM2LONG(argv[3]) : 24;
	if (segments < 3)
		rb_raise(rb_eArgError, "a circle needs at least 3 segments");
	return add_arc_curve(self, su_to_point3d(argv[0]), xaxis, normal, NUM2DBL(argv[2]), 0.0, 2 * M_PI, segments);
}

static VALUE entities_add_ngon(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 3, 4);
	struct SUPoint3D center = su_to_point3d(argv[0]);
	struct SUVector3D normal = su_to_vector3d(argv[1]);
	double radius = NUM2DBL(argv[2]);
	long sides = argc == 4 ? NUM2LONG(argv[3]) : 24;
	if (sides < 3)
		rb_raise(rb_eArgError, "a polygon needs at least 3 sides");
	if (radius <= 0)
		rb_raise(rb_eArgError, "radius must be positive");
	struct SUVector3D x;
	struct SUVector3D y;
	if (SUVector3DGetArbitraryAxes(&normal, &x, &y) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "invalid normal");
	VALUE points = rb_ary_new_capa(sides + 1);
	for (long i = 0; i <= sides; i++)
	{
		double angle = 2 * M_PI * (double)(i % sides) / (double)sides;
		double c = cos(angle) * radius;
		double s = sin(angle) * radius;
		rb_ary_push(points, su_point3d_new((struct SUPoint3D){center.x + x.x * c + y.x * s, center.y + x.y * c + y.y * s,
			center.z + x.z * c + y.z * s}));
	}
	VALUE args[1] = {points};
	return entities_add_edges(1, args, self);
}

static VALUE entities_add_curve(int argc, VALUE* argv, VALUE self)
{
	VALUE edges = entities_add_edges(argc, argv, self);
	long count = RARRAY_LEN(edges);
	SUEdgeRef* refs = ALLOC_N(SUEdgeRef, count);
	for (long i = 0; i < count; i++)
		refs[i].ptr = DATA_PTR(RARRAY_AREF(edges, i));
	SUCurveRef curve = SU_INVALID;
	enum SUResult result = SUCurveCreateWithEdges(&curve, refs, (size_t)count);
	xfree(refs);
	if (result != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "cannot create curve");
	SUEntitiesAddCurves(ENTITIES(self), 1, &curve);
	return curve_edges(curve);
}

static VALUE entities_add_cpoint(VALUE self, VALUE point)
{
	struct SUPoint3D p = su_to_point3d_like(point);
	SUGuidePointRef guide = SU_INVALID;
	SUGuidePointCreate(&guide, &p);
	SUEntitiesAddGuidePoints(ENTITIES(self), 1, &guide);
	return su_wrap(cConstructionPoint, guide.ptr);
}

static VALUE entities_add_cline(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 2, 3);
	if (argc == 3)
		rb_raise(rb_eNotImpError, "the C API cannot set a construction line stipple");
	struct SUPoint3D start = su_to_point3d(argv[0]);
	SUGuideLineRef guide = SU_INVALID;
	if (rb_obj_is_kind_of(argv[1], cVector3d))
	{
		struct SUVector3D direction = su_to_vector3d(argv[1]);
		SUGuideLineCreateInfinite(&guide, &start, &direction);
	}
	else
	{
		struct SUPoint3D end = su_to_point3d(argv[1]);
		SUGuideLineCreateFinite(&guide, &start, &end);
	}
	if (SUIsInvalid(guide))
		rb_raise(rb_eArgError, "cannot create construction line");
	SUEntitiesAddGuideLines(ENTITIES(self), 1, &guide);
	return su_wrap(cConstructionLine, guide.ptr);
}

static VALUE entities_add_group(int argc, VALUE* argv, VALUE self)
{
	if (argc > 0 && RARRAY_LEN(varargs(argc, argv)) > 0)
		rb_raise(rb_eNotImpError, "the C API cannot move existing entities into a group");
	SUGroupRef group = SU_INVALID;
	SUGroupCreate(&group);
	SUEntitiesAddGroup(ENTITIES(self), group);
	return su_wrap(cGroup, group.ptr);
}

static VALUE entities_add_instance(VALUE self, VALUE definition, VALUE transformation)
{
	SUComponentDefinitionRef def = {su_unwrap(definition, cComponentDefinition)};
	struct SUTransformation t = rb_obj_is_kind_of(transformation, cTransformation)
		? su_to_transformation(transformation)
		: su_to_transformation(rb_class_new_instance(1, &transformation, cTransformation));
	SUComponentInstanceRef instance = SU_INVALID;
	if (SUComponentDefinitionCreateInstance(def, &instance) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "cannot create an instance of this definition");
	SUComponentInstanceSetTransform(instance, &t);
	SUEntitiesAddInstance(ENTITIES(self), instance, NULL);
	return su_wrap(cComponentInstance, instance.ptr);
}

static VALUE entities_add_image(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 3, 4);
	SUImageRef image = SU_INVALID;
	if (SUImageCreateFromFile(&image, StringValueCStr(argv[0])) != SU_ERROR_NONE)
		return Qnil;
	double native_width = 1.0;
	double native_height = 1.0;
	SUImageGetDimensions(image, &native_width, &native_height);
	double width = NUM2DBL(argv[2]);
	double height = argc == 4 && !NIL_P(argv[3]) ? NUM2DBL(argv[3]) : width * native_height / native_width;
	struct SUPoint3D origin = su_to_point3d(argv[1]);
	struct SUTransformation t = {{width / native_width, 0, 0, 0, 0, height / native_height, 0, 0, 0, 0, 1, 0,
		origin.x, origin.y, origin.z, 1}};
	SUImageSetTransform(image, &t);
	SUEntitiesAddImage(ENTITIES(self), image);
	return su_wrap(cImage, image.ptr);
}

static VALUE entities_add_section_plane(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 1, 2);
	struct SUPlane3D plane = su_to_plane3d(argc == 2 ? rb_ary_new_from_values(2, argv) : argv[0]);
	SUSectionPlaneRef section = SU_INVALID;
	SUSectionPlaneCreate(&section);
	SUSectionPlaneSetPlane(section, &plane);
	SUEntitiesAddSectionPlanes(ENTITIES(self), 1, &section);
	return su_wrap(cSectionPlane, section.ptr);
}

static VALUE entities_add_text(VALUE self, VALUE string, VALUE point, VALUE vector)
{
	struct SUPoint3D p = su_to_point3d_like(point);
	struct SUVector3D v = su_to_vector3d(vector);
	SUTextRef text = SU_INVALID;
	SUTextCreate(&text);
	SUTextSetString(text, StringValueCStr(string));
	SUInstancePathRef no_path = SU_INVALID;
	SUTextSetPoint(text, &p, no_path);
	SUTextSetLeaderVector(text, &v);
	SUEntitiesAddTexts(ENTITIES(self), 1, &text);
	return su_wrap(cText, text.ptr);
}

static SUMaterialRef material_from(VALUE self, VALUE material)
{
	SUMaterialRef ref = SU_INVALID;
	if (NIL_P(material))
		return ref;
	if (rb_obj_is_kind_of(material, cMaterial))
		ref.ptr = DATA_PTR(material);
	else
	{
		VALUE found = rb_funcall(rb_funcall(entities_model(self), (rb_intern)("materials"), 0), (rb_intern)("[]"), 1, material);
		if (NIL_P(found))
			rb_raise(rb_eArgError, "material not found: %"PRIsVALUE, material);
		ref.ptr = DATA_PTR(found);
	}
	return ref;
}

static VALUE fill_from_mesh(VALUE self, VALUE mesh, bool weld, int smooth_flags, VALUE front, VALUE back)
{
	su_unwrap(mesh, cPolygonMesh);
	VALUE points = rb_funcall(mesh, (rb_intern)("points"), 0);
	VALUE polygons = rb_funcall(mesh, (rb_intern)("polygons"), 0);
	SUMaterialRef front_material = material_from(self, front);
	SUMaterialRef back_material = material_from(self, back);
	SUGeometryInputRef input = SU_INVALID;
	SUGeometryInputCreate(&input);
	for (long i = 0; i < RARRAY_LEN(points); i++)
	{
		struct SUPoint3D p = su_to_point3d(RARRAY_AREF(points, i));
		SUGeometryInputAddVertex(input, &p);
	}
	for (long i = 0; i < RARRAY_LEN(polygons); i++)
	{
		VALUE polygon = RARRAY_AREF(polygons, i);
		SULoopInputRef loop = SU_INVALID;
		SULoopInputCreate(&loop);
		for (long j = 0; j < RARRAY_LEN(polygon); j++)
		{
			long index = NUM2LONG(RARRAY_AREF(polygon, j));
			SULoopInputAddVertexIndex(loop, (size_t)(labs(index) - 1));
			if (index < 0 && (smooth_flags & 1))
				SULoopInputEdgeSetHidden(loop, (size_t)j, true);
		}
		size_t face_index = 0;
		SUGeometryInputAddFace(input, &loop, &face_index);
		if (SUIsValid(front_material))
		{
			struct SUMaterialInput material = {0, {{0, 0}}, {0}, front_material};
			SUGeometryInputFaceSetFrontMaterial(input, face_index, &material);
		}
		if (SUIsValid(back_material))
		{
			struct SUMaterialInput material = {0, {{0, 0}}, {0}, back_material};
			SUGeometryInputFaceSetBackMaterial(input, face_index, &material);
		}
	}
	size_t faces_before = 0;
	SUEntitiesGetNumFaces(ENTITIES(self), &faces_before);
	enum SUResult result = SUEntitiesFill(ENTITIES(self), input, weld);
	SUGeometryInputRelease(&input);
	if (result != SU_ERROR_NONE)
		return Qfalse;
	size_t faces_after = 0;
	SUEntitiesGetNumFaces(ENTITIES(self), &faces_after);
	return SIZET2NUM(faces_after - faces_before);
}

static VALUE entities_add_faces_from_mesh(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 1, 4);
	int flags = argc > 1 ? NUM2INT(argv[1]) : 12;
	return fill_from_mesh(self, argv[0], true, flags, argc > 2 ? argv[2] : Qnil, argc > 3 ? argv[3] : Qnil);
}

static VALUE entities_fill_from_mesh(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 1, 5);
	bool weld = argc > 1 ? RTEST(argv[1]) : true;
	int flags = argc > 2 ? NUM2INT(argv[2]) : 12;
	VALUE added = fill_from_mesh(self, argv[0], weld, flags, argc > 3 ? argv[3] : Qnil, argc > 4 ? argv[4] : Qnil);
	return BOOL2RB(added != Qfalse);
}

static SUEntityRef* entity_refs(VALUE entities, long* count)
{
	*count = RARRAY_LEN(entities);
	SUEntityRef* refs = ALLOC_N(SUEntityRef, *count > 0 ? *count : 1);
	for (long i = 0; i < *count; i++)
		refs[i].ptr = su_unwrap(RARRAY_AREF(entities, i), cEntity);
	return refs;
}

static VALUE entities_erase_entities(int argc, VALUE* argv, VALUE self)
{
	long count;
	SUEntityRef* refs = entity_refs(varargs(argc, argv), &count);
	enum SUResult result = SUEntitiesErase(ENTITIES(self), (size_t)count, refs);
	xfree(refs);
	if (result != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "entities must belong to this collection");
	return Qnil;
}

static VALUE entities_clear_bang(VALUE self)
{
	return BOOL2RB(SUEntitiesClear(ENTITIES(self)) == SU_ERROR_NONE);
}

static VALUE entities_transform_entities(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 2, UNLIMITED_ARGUMENTS);
	struct SUTransformation t = su_to_transformation(argv[0]);
	long count;
	SUEntityRef* refs = entity_refs(varargs(argc - 1, argv + 1), &count);
	enum SUResult result = SUEntitiesTransform(ENTITIES(self), (size_t)count, refs, &t);
	xfree(refs);
	return BOOL2RB(result == SU_ERROR_NONE);
}

static VALUE entities_transform_by_vectors(VALUE self, VALUE entities, VALUE vectors)
{
	Check_Type(vectors, T_ARRAY);
	long count;
	SUEntityRef* refs = entity_refs(entities, &count);
	if (RARRAY_LEN(vectors) != count)
	{
		xfree(refs);
		rb_raise(rb_eArgError, "expected one vector per entity");
	}
	struct SUTransformation* transforms = ALLOC_N(struct SUTransformation, count > 0 ? count : 1);
	for (long i = 0; i < count; i++)
		transforms[i] = su_to_transformation(rb_class_new_instance(1, &RARRAY_CONST_PTR(vectors)[i], cTransformation));
	SUEntitiesTransformMultiple(ENTITIES(self), (size_t)count, refs, transforms);
	xfree(refs);
	xfree(transforms);
	return self;
}

static VALUE entities_weld(VALUE self, VALUE edges)
{
	Check_Type(edges, T_ARRAY);
	long count = RARRAY_LEN(edges);
	SUEdgeRef* refs = ALLOC_N(SUEdgeRef, count > 0 ? count : 1);
	for (long i = 0; i < count; i++)
		refs[i].ptr = su_unwrap(RARRAY_AREF(edges, i), cEdge);
	SUEntityListRef list = SU_INVALID;
	SUEntityListCreate(&list);
	SUEntitiesWeld(ENTITIES(self), (size_t)count, refs, list);
	xfree(refs);
	VALUE curves = rb_ary_new();
	SUEntityListIteratorRef it = SU_INVALID;
	SUEntityListIteratorCreate(&it);
	SUEntityListBegin(list, &it);
	bool in_range = false;
	while (SUEntityListIteratorIsInRange(it, &in_range) == SU_ERROR_NONE && in_range)
	{
		SUEntityRef entity = SU_INVALID;
		SUEntityListIteratorGetEntity(it, &entity);
		rb_ary_push(curves, su_wrap(cCurve, entity.ptr));
		SUEntityListIteratorNext(it);
	}
	SUEntityListIteratorRelease(&it);
	SUEntityListRelease(&list);
	return curves;
}

static VALUE entities_active_section_plane(VALUE self)
{
	SUSectionPlaneRef section = SU_INVALID;
	SUEntitiesGetActiveSectionPlane(ENTITIES(self), &section);
	return su_wrap_or_nil(cSectionPlane, section.ptr);
}

static VALUE entities_set_active_section_plane(VALUE self, VALUE section)
{
	SUSectionPlaneRef ref = SU_INVALID;
	if (!NIL_P(section))
		ref.ptr = su_unwrap(section, cSectionPlane);
	SUEntitiesSetActiveSectionPlane(ENTITIES(self), ref);
	return section;
}

void Init_entities(void)
{
	rb_include_module(cEntities, rb_mEnumerable);
	rb_define_method(cEntities, "each", entities_each, 0);
	rb_define_method(cEntities, "[]", entities_aref, 1);
	rb_define_method(cEntities, "at", entities_aref, 1);
	rb_define_method(cEntities, "count", entities_length, 0);
	rb_define_method(cEntities, "length", entities_length, 0);
	rb_define_method(cEntities, "size", entities_length, 0);
	rb_define_method(cEntities, "model", entities_model, 0);
	rb_define_method(cEntities, "parent", entities_parent, 0);
	rb_define_method(cEntities, "==", entities_eq, 1);
	rb_define_method(cEntities, "add_line", entities_add_line, 2);
	rb_define_method(cEntities, "add_edges", entities_add_edges, -1);
	rb_define_method(cEntities, "add_face", entities_add_face, -1);
	rb_define_method(cEntities, "add_arc", entities_add_arc, -1);
	rb_define_method(cEntities, "add_circle", entities_add_circle, -1);
	rb_define_method(cEntities, "add_ngon", entities_add_ngon, -1);
	rb_define_method(cEntities, "add_curve", entities_add_curve, -1);
	rb_define_method(cEntities, "add_cpoint", entities_add_cpoint, 1);
	rb_define_method(cEntities, "add_cline", entities_add_cline, -1);
	rb_define_method(cEntities, "add_group", entities_add_group, -1);
	rb_define_method(cEntities, "add_instance", entities_add_instance, 2);
	rb_define_method(cEntities, "add_image", entities_add_image, -1);
	rb_define_method(cEntities, "add_section_plane", entities_add_section_plane, -1);
	rb_define_method(cEntities, "add_text", entities_add_text, 3);
	rb_define_method(cEntities, "add_faces_from_mesh", entities_add_faces_from_mesh, -1);
	rb_define_method(cEntities, "fill_from_mesh", entities_fill_from_mesh, -1);
	rb_define_method(cEntities, "erase_entities", entities_erase_entities, -1);
	rb_define_method(cEntities, "clear!", entities_clear_bang, 0);
	rb_define_method(cEntities, "transform_entities", entities_transform_entities, -1);
	rb_define_method(cEntities, "transform_by_vectors", entities_transform_by_vectors, 2);
	rb_define_method(cEntities, "weld", entities_weld, 1);
	rb_define_method(cEntities, "active_section_plane", entities_active_section_plane, 0);
	rb_define_method(cEntities, "active_section_plane=", entities_set_active_section_plane, 1);
}
