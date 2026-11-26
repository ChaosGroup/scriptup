#include <scriptup.h>

VALUE mSketchup;
VALUE mGeom;

#define SCRIPTUP_DEFINE_CLASS(var, ns, name, parent) VALUE var;
SCRIPTUP_CLASSES(SCRIPTUP_DEFINE_CLASS)

static SUModelRef active_model = SU_INVALID;

SUModelRef Sketchup_get_active_model(void)
{
	return active_model;
}

void Sketchup_set_active_model(SUModelRef model)
{
	active_model = model;
}

static VALUE sketchup_open_file(int argc, VALUE* argv, VALUE self)
{
	VALUE path;
	VALUE options;
	rb_scan_args(argc, argv, "1:", &path, &options);

	bool with_status = true;
	if (!NIL_P(options))
	{
		ID ids[1] = { (rb_intern)("with_status") };
		VALUE values[1];
		rb_get_kwargs(options, ids, 0, 1, values);
		if (values[0] != Qundef)
			with_status = RTEST(values[0]);
	}

	enum SUModelLoadStatus status;
	SUModelRef model = SU_INVALID;
	if (SUModelCreateFromFileWithStatus(&model, StringValueCStr(path), &status) != SU_ERROR_NONE)
		return Qfalse;

	Sketchup_set_active_model(model);
	return with_status ? INT2FIX(status) : Qtrue;
}

static VALUE sketchup_active_model(VALUE self)
{
	return su_wrap_or_nil(cModel, active_model.ptr);
}

void Init_sketchup(void)
{
	SUInitialize();
	mSketchup = rb_define_module("Sketchup");
	mGeom = rb_define_module("Geom");

#define SCRIPTUP_CREATE_CLASS(var, ns, name, parent) \
	var = rb_define_class_under(ns, name, parent); \
	rb_undef_alloc_func(var);
	SCRIPTUP_CLASSES(SCRIPTUP_CREATE_CLASS)

	rb_define_singleton_method(mSketchup, "open_file", sketchup_open_file, -1);
	rb_define_singleton_method(mSketchup, "active_model", sketchup_active_model, 0);

	Init_arc_curve();
	Init_attribute_dictionaries();
	Init_attribute_dictionary();
	Init_bounding_box();
	Init_color();
	Init_component_definition();
	Init_curve();
	Init_definition_list();
	Init_drawing_element();
	Init_entities();
	Init_edge();
	Init_edge_use();
	Init_entity();
	Init_face();
	Init_geom();
	Init_lat_long();
	Init_length();
	Init_loop();
	Init_material();
	Init_materials();
	Init_model();
	Init_point2d();
	Init_point3d();
	Init_polygon_mesh();
	Init_texture();
	Init_transformation();
	Init_transformation2d();
	Init_vector2d();
	Init_vector3d();
	Init_vertex();
}
