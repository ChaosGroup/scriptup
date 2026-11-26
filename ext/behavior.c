#include <scriptup.h>

// A Behavior wraps its component definition; every change is written back.
static struct SUComponentBehavior behavior_get(VALUE self)
{
	struct SUComponentBehavior behavior;
	memset(&behavior, 0, sizeof(behavior));
	SUComponentDefinitionGetBehavior(SU_REF(SUComponentDefinitionRef, self), &behavior);
	return behavior;
}

static void behavior_set(VALUE self, struct SUComponentBehavior behavior)
{
	SUComponentDefinitionSetBehavior(SU_REF(SUComponentDefinitionRef, self), &behavior);
}

#define DEFINE_FLAG(name, field) \
	static VALUE behavior_##name##_p(VALUE self) { return BOOL2RB(behavior_get(self).field); } \
	static VALUE behavior_set_##name(VALUE self, VALUE value) \
	{ \
		struct SUComponentBehavior b = behavior_get(self); \
		b.field = RTEST(value); \
		behavior_set(self, b); \
		return value; \
	}
DEFINE_FLAG(always_face_camera, component_always_face_camera)
DEFINE_FLAG(cuts_opening, component_cuts_opening)
DEFINE_FLAG(shadows_face_sun, component_shadows_face_sun)

static VALUE behavior_no_scale_mask_p(VALUE self)
{
	return SIZET2NUM(behavior_get(self).component_no_scale_mask);
}

static VALUE behavior_set_no_scale_mask(VALUE self, VALUE mask)
{
	struct SUComponentBehavior b = behavior_get(self);
	b.component_no_scale_mask = NUM2SIZET(mask) & 0x7F;
	behavior_set(self, b);
	return mask;
}

// Ruby SnapTo_Arbitrary/Horizontal/Vertical/Sloped are 0..3; the C enum adds None in front.
static VALUE behavior_is2d_p(VALUE self)
{
	return BOOL2RB(behavior_get(self).component_snap != SUSnapToBehavior_None);
}

static VALUE behavior_set_is2d(VALUE self, VALUE value)
{
	struct SUComponentBehavior b = behavior_get(self);
	if (!RTEST(value))
		b.component_snap = SUSnapToBehavior_None;
	else if (b.component_snap == SUSnapToBehavior_None)
		b.component_snap = SUSnapToBehavior_Any;
	behavior_set(self, b);
	return value;
}

static VALUE behavior_snapto(VALUE self)
{
	enum SUSnapToBehavior snap = behavior_get(self).component_snap;
	return INT2FIX(snap == SUSnapToBehavior_None ? 0 : (int)snap - 1);
}

static VALUE behavior_set_snapto(VALUE self, VALUE value)
{
	int snapto = NUM2INT(value);
	if (snapto < 0 || snapto > 3)
		rb_raise(rb_eArgError, "snapto must be between 0 and 3");
	struct SUComponentBehavior b = behavior_get(self);
	b.component_snap = (enum SUSnapToBehavior)(snapto + 1);
	behavior_set(self, b);
	return value;
}

void Init_behavior(void)
{
	rb_define_method(cBehavior, "always_face_camera?", behavior_always_face_camera_p, 0);
	rb_define_method(cBehavior, "always_face_camera=", behavior_set_always_face_camera, 1);
	rb_define_method(cBehavior, "cuts_opening?", behavior_cuts_opening_p, 0);
	rb_define_method(cBehavior, "cuts_opening=", behavior_set_cuts_opening, 1);
	rb_define_method(cBehavior, "shadows_face_sun?", behavior_shadows_face_sun_p, 0);
	rb_define_method(cBehavior, "shadows_face_sun=", behavior_set_shadows_face_sun, 1);
	rb_define_method(cBehavior, "no_scale_mask?", behavior_no_scale_mask_p, 0);
	rb_define_method(cBehavior, "no_scale_mask=", behavior_set_no_scale_mask, 1);
	rb_define_method(cBehavior, "is2d?", behavior_is2d_p, 0);
	rb_define_method(cBehavior, "is2d=", behavior_set_is2d, 1);
	rb_define_method(cBehavior, "snapto", behavior_snapto, 0);
	rb_define_method(cBehavior, "snapto=", behavior_set_snapto, 1);
}
