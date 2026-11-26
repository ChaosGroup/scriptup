#include <scriptup.h>

#define SECTION(self) SU_REF(SUSectionPlaneRef, self)

DEFINE_STRING_GETTER(sectionplane_name, SUSectionPlaneRef, SUSectionPlaneGetName)
DEFINE_STRING_SETTER(sectionplane_set_name, SUSectionPlaneRef, SUSectionPlaneSetName)
DEFINE_STRING_GETTER(sectionplane_symbol, SUSectionPlaneRef, SUSectionPlaneGetSymbol)
DEFINE_STRING_SETTER(sectionplane_set_symbol, SUSectionPlaneRef, SUSectionPlaneSetSymbol)
DEFINE_BOOL_GETTER(sectionplane_active_p, SUSectionPlaneRef, SUSectionPlaneIsActive)

static VALUE sectionplane_get_plane(VALUE self)
{
	struct SUPlane3D plane = {0, 0, 1, 0};
	SUSectionPlaneGetPlane(SECTION(self), &plane);
	return rb_ary_new_from_args(4, DBL2NUM(plane.a), DBL2NUM(plane.b), DBL2NUM(plane.c), DBL2NUM(plane.d));
}

static VALUE sectionplane_set_plane(VALUE self, VALUE plane)
{
	struct SUPlane3D p = su_to_plane3d(plane);
	SUSectionPlaneSetPlane(SECTION(self), &p);
	return self;
}

static VALUE sectionplane_activate(VALUE self)
{
	SUEntitiesRef parent = SU_INVALID;
	SUEntityGetParentEntities(SU_REF(SUEntityRef, self), &parent);
	if (SUEntitiesSetActiveSectionPlane(parent, SECTION(self)) != SU_ERROR_NONE)
		rb_raise(rb_eRuntimeError, "cannot activate this section plane");
	return self;
}

void Init_section_plane(void)
{
	rb_define_method(cSectionPlane, "name", sectionplane_name, 0);
	rb_define_method(cSectionPlane, "name=", sectionplane_set_name, 1);
	rb_define_method(cSectionPlane, "symbol", sectionplane_symbol, 0);
	rb_define_method(cSectionPlane, "symbol=", sectionplane_set_symbol, 1);
	rb_define_method(cSectionPlane, "active?", sectionplane_active_p, 0);
	rb_define_method(cSectionPlane, "activate", sectionplane_activate, 0);
	rb_define_method(cSectionPlane, "get_plane", sectionplane_get_plane, 0);
	rb_define_method(cSectionPlane, "set_plane", sectionplane_set_plane, 1);
}
