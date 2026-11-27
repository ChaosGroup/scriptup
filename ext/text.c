#include <scriptup.h>

#define TEXT_REF(self) SU_REF(SUTextRef, self)

DEFINE_STRING_GETTER(text_text, SUTextRef, SUTextGetString)
DEFINE_STRING_SETTER(text_set_text, SUTextRef, SUTextSetString)
DEFINE_ENUM_GETTER(text_leader_type, SUTextRef, SUTextGetLeaderType, enum SUTextLeaderType)
DEFINE_ENUM_GETTER(text_arrow_type, SUTextRef, SUTextGetArrowType, enum SUArrowType)

static VALUE text_set_text_bang(VALUE self, VALUE string)
{
	text_set_text(self, string);
	return self;
}

static VALUE text_set_leader_type(VALUE self, VALUE type)
{
	int t = NUM2INT(type);
	if (t < SUTextLeaderType_None || t > SUTextLeaderType_PushPin)
		rb_raise(rb_eArgError, "invalid leader type");
	SUTextSetLeaderType(TEXT_REF(self), (enum SUTextLeaderType)t);
	return type;
}

static VALUE text_set_arrow_type(VALUE self, VALUE type)
{
	int t = NUM2INT(type);
	if (t < SUArrowNone || t > SUArrowOpen)
		rb_raise(rb_eArgError, "invalid arrow type");
	SUTextSetArrowType(TEXT_REF(self), (enum SUArrowType)t);
	return type;
}

static VALUE text_has_leader_p(VALUE self)
{
	enum SUTextLeaderType type = SUTextLeaderType_None;
	SUTextGetLeaderType(TEXT_REF(self), &type);
	return BOOL2RB(type != SUTextLeaderType_None);
}

static VALUE text_set_display_leader(VALUE self, VALUE display)
{
	enum SUTextLeaderType type = SUTextLeaderType_None;
	SUTextGetLeaderType(TEXT_REF(self), &type);
	if (!RTEST(display))
		type = SUTextLeaderType_None;
	else if (type == SUTextLeaderType_None)
		type = SUTextLeaderType_ViewBased;
	SUTextSetLeaderType(TEXT_REF(self), type);
	return display;
}

static VALUE text_point(VALUE self)
{
	struct SUPoint3D p = {0, 0, 0};
	SUInstancePathRef path = SU_INVALID;
	SUTextGetPoint(TEXT_REF(self), &p, &path);
	return su_point3d_new(p);
}

static VALUE text_set_point(VALUE self, VALUE point)
{
	struct SUPoint3D p = su_to_point3d_like(point);
	SUInstancePathRef none = SU_INVALID;
	SUTextSetPoint(TEXT_REF(self), &p, none);
	return point;
}

static VALUE text_attached_to(VALUE self)
{
	struct SUPoint3D p = {0, 0, 0};
	SUInstancePathRef path = SU_INVALID;
	SUTextGetPoint(TEXT_REF(self), &p, &path);
	if (SUIsInvalid(path))
		return Qnil;
	VALUE copy = su_instance_path_copy(path);
	if (NIL_P(copy) || RTEST(rb_funcall(copy, (rb_intern)("empty?"), 0)))
		return Qnil;
	return rb_ary_new_from_args(2, copy, su_point3d_new(p));
}

static VALUE text_set_attached_to(VALUE self, VALUE attachment)
{
	Check_Type(attachment, T_ARRAY);
	if (RARRAY_LEN(attachment) != 2)
		rb_raise(rb_eArgError, "expected [instance_path, point]");
	SUInstancePathRef path = su_to_instance_path(RARRAY_AREF(attachment, 0));
	struct SUPoint3D p = su_to_point3d(RARRAY_AREF(attachment, 1));
	if (SUTextSetPoint(TEXT_REF(self), &p, path) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "cannot attach text to this path");
	return attachment;
}

static VALUE text_vector(VALUE self)
{
	struct SUVector3D v = {0, 0, 0};
	SUTextGetLeaderVector(TEXT_REF(self), &v);
	return su_vector3d_new(v);
}

static VALUE text_set_vector(VALUE self, VALUE vector)
{
	struct SUVector3D v = su_to_vector3d(vector);
	SUTextSetLeaderVector(TEXT_REF(self), &v);
	return vector;
}

void Init_text(void)
{
	rb_define_const(rb_cObject, "ALeaderNone", INT2FIX(SUTextLeaderType_None));
	rb_define_const(rb_cObject, "ALeaderView", INT2FIX(SUTextLeaderType_ViewBased));
	rb_define_const(rb_cObject, "ALeaderModel", INT2FIX(SUTextLeaderType_PushPin));
	static const struct { const char* name; int value; } arrows[] = {
		{"ARROW_NONE", SUArrowNone}, {"ARROW_SLASH", SUArrowSlash}, {"ARROW_DOT", SUArrowDot},
		{"ARROW_CLOSED", SUArrowClosed}, {"ARROW_OPEN", SUArrowOpen},
	};
	for (size_t i = 0; i < sizeof(arrows) / sizeof(arrows[0]); i++)
		rb_define_const(cDimension, arrows[i].name, INT2FIX(arrows[i].value));
	rb_define_method(cText, "text", text_text, 0);
	rb_define_method(cText, "text=", text_set_text, 1);
	rb_define_method(cText, "set_text", text_set_text_bang, 1);
	rb_define_method(cText, "leader_type", text_leader_type, 0);
	rb_define_method(cText, "leader_type=", text_set_leader_type, 1);
	rb_define_method(cText, "arrow_type", text_arrow_type, 0);
	rb_define_method(cText, "arrow_type=", text_set_arrow_type, 1);
	rb_define_method(cText, "has_leader?", text_has_leader_p, 0);
	rb_define_method(cText, "display_leader?", text_has_leader_p, 0);
	rb_define_method(cText, "display_leader=", text_set_display_leader, 1);
	rb_define_method(cText, "point", text_point, 0);
	rb_define_method(cText, "point=", text_set_point, 1);
	rb_define_method(cText, "attached_to", text_attached_to, 0);
	rb_define_method(cText, "attached_to=", text_set_attached_to, 1);
	rb_define_method(cText, "vector", text_vector, 0);
	rb_define_method(cText, "vector=", text_set_vector, 1);
}
