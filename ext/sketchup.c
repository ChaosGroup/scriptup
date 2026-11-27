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

// Creates an empty model and makes it the active one.
static VALUE sketchup_file_new(VALUE self)
{
	SUModelRef model = SU_INVALID;
	if (SUModelCreate(&model) != SU_ERROR_NONE)
		rb_raise(rb_eRuntimeError, "cannot create a model");
	Sketchup_set_active_model(model);
	return self;
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
	rb_define_singleton_method(mSketchup, "file_new", sketchup_file_new, 0);

	Init_arc_curve();
	Init_attribute_dictionaries();
	Init_attribute_dictionary();
	Init_axes();
	Init_behavior();
	Init_bounding_box();
	Init_camera();
	Init_classifications();
	Init_color();
	Init_component_definition();
	Init_component_instance();
	Init_construction();
	Init_curve();
	Init_definition_list();
	Init_drawing_element();
	Init_entities();
	Init_entities_builder();
	Init_edge();
	Init_edge_use();
	Init_entity();
	Init_environments();
	Init_face();
	Init_geom();
	Init_group();
	Init_image();
	Init_image_rep();
	Init_instance_path();
	Init_lat_long();
	Init_layer();
	Init_layer_folder();
	Init_layers();
	Init_length();
	Init_line_styles();
	Init_loop();
	Init_material();
	Init_materials();
	Init_model();
	Init_options_store();
	Init_page();
	Init_pages();
	Init_point2d();
	Init_point3d();
	Init_polygon_mesh();
	Init_section_plane();
	Init_styles();
	Init_text();
	Init_texture();
	Init_texture_writer();
	Init_transformation();
	Init_transformation2d();
	Init_uv_helper();
	Init_vector2d();
	Init_vector3d();
	Init_vertex();
}
