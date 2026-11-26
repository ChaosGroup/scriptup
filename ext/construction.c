#include <scriptup.h>

static VALUE constructionpoint_position(VALUE self)
{
	struct SUPoint3D p = {0, 0, 0};
	SUGuidePointGetPosition(SU_REF(SUGuidePointRef, self), &p);
	return su_point3d_new(p);
}

struct guide_line
{
	struct SUPoint3D start;
	struct SUVector3D direction;
	bool infinite;
};

static struct guide_line guide_line_data(VALUE self)
{
	struct guide_line data = {{0, 0, 0}, {0, 0, 0}, false};
	SUGuideLineGetData(SU_REF(SUGuideLineRef, self), &data.start, &data.direction, &data.infinite);
	return data;
}

static VALUE constructionline_position(VALUE self)
{
	return su_point3d_new(guide_line_data(self).start);
}

static VALUE constructionline_direction(VALUE self)
{
	struct SUVector3D v = guide_line_data(self).direction;
	SUVector3DNormalize(&v);
	return su_vector3d_new(v);
}

static VALUE constructionline_start(VALUE self)
{
	struct guide_line data = guide_line_data(self);
	return data.infinite ? Qnil : su_point3d_new(data.start);
}

static VALUE constructionline_end(VALUE self)
{
	struct guide_line data = guide_line_data(self);
	if (data.infinite)
		return Qnil;
	struct SUPoint3D end;
	SUPoint3DOffset(&data.start, &data.direction, &end);
	return su_point3d_new(end);
}

void Init_construction(void)
{
	rb_define_method(cConstructionPoint, "position", constructionpoint_position, 0);
	rb_define_method(cConstructionLine, "position", constructionline_position, 0);
	rb_define_method(cConstructionLine, "direction", constructionline_direction, 0);
	rb_define_method(cConstructionLine, "start", constructionline_start, 0);
	rb_define_method(cConstructionLine, "end", constructionline_end, 0);
}
