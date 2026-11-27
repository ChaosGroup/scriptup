#include <scriptup.h>

#define CAMERA(self) SU_REF(SUCameraRef, self)

static void camera_free(void* ptr)
{
	SUCameraRef camera = {ptr};
	if (ptr)
		SUCameraRelease(&camera);
}

static VALUE camera_alloc(VALUE klass)
{
	SUCameraRef camera = SU_INVALID;
	SUCameraCreate(&camera);
	return Data_Wrap_Struct(klass, 0, camera_free, camera.ptr);
}

// Wraps a camera owned by the model or a page.
VALUE su_camera_wrap(SUCameraRef camera)
{
	return su_wrap_or_nil(cCamera, camera.ptr);
}

struct orientation
{
	struct SUPoint3D eye;
	struct SUPoint3D target;
	struct SUVector3D up;
};

static struct orientation camera_orientation(VALUE self)
{
	struct orientation o = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
	SUCameraGetOrientation(CAMERA(self), &o.eye, &o.target, &o.up);
	return o;
}

static VALUE camera_set(VALUE self, VALUE eye, VALUE target, VALUE up)
{
	struct SUPoint3D e = su_to_point3d(eye);
	struct SUPoint3D t = su_to_point3d(target);
	struct SUVector3D u = su_to_vector3d(up);
	bool same = false;
	SUPoint3DGetEqual(&e, &t, &same);
	if (same)
		rb_raise(rb_eArgError, "eye and target must differ");
	if (SUCameraSetOrientation(CAMERA(self), &e, &t, &u) != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "invalid camera orientation");
	return self;
}

static VALUE camera_initialize(int argc, VALUE* argv, VALUE self)
{
	VALUE eye, target, up, perspective, fov;
	rb_scan_args(argc, argv, "05", &eye, &target, &up, &perspective, &fov);
	if (argc == 0)
		return self;
	if (argc < 3)
		rb_raise(rb_eArgError, "wrong number of arguments (given %d, expected 0 or 3..5)", argc);
	camera_set(self, eye, target, up);
	SUCameraSetPerspective(CAMERA(self), NIL_P(perspective) || RTEST(perspective));
	if (!NIL_P(fov))
		SUCameraSetPerspectiveFrustumFOV(CAMERA(self), NUM2DBL(fov));
	return self;
}

static VALUE camera_eye(VALUE self) { return su_point3d_new(camera_orientation(self).eye); }
static VALUE camera_target(VALUE self) { return su_point3d_new(camera_orientation(self).target); }
static VALUE camera_up(VALUE self) { return su_vector3d_new(camera_orientation(self).up); }

static VALUE camera_direction(VALUE self)
{
	struct SUVector3D v = {0, 0, 0};
	SUCameraGetDirection(CAMERA(self), &v);
	return su_vector3d_new(v);
}

// Camera space: z along the view direction, y up, x to the right.
static VALUE camera_zaxis(VALUE self) { return camera_direction(self); }

static VALUE camera_xaxis(VALUE self)
{
	struct SUVector3D z = {0, 0, 0};
	SUCameraGetDirection(CAMERA(self), &z);
	struct SUVector3D up = camera_orientation(self).up;
	struct SUVector3D x;
	SUVector3DCross(&z, &up, &x);
	SUVector3DNormalize(&x);
	return su_vector3d_new(x);
}

static VALUE camera_yaxis(VALUE self)
{
	struct SUVector3D z = {0, 0, 0};
	SUCameraGetDirection(CAMERA(self), &z);
	struct SUVector3D x = su_to_vector3d(camera_xaxis(self));
	struct SUVector3D y;
	SUVector3DCross(&x, &z, &y);
	SUVector3DNormalize(&y);
	return su_vector3d_new(y);
}

DEFINE_STRING_GETTER(camera_description, SUCameraRef, SUCameraGetDescription)
DEFINE_STRING_SETTER(camera_set_description, SUCameraRef, SUCameraSetDescription)
DEFINE_DOUBLE_GETTER(camera_fov, SUCameraRef, SUCameraGetPerspectiveFrustumFOV)
DEFINE_SETTER(camera_set_fov, SUCameraRef, SUCameraSetPerspectiveFrustumFOV, NUM2DBL)
DEFINE_DOUBLE_GETTER(camera_aspect_ratio, SUCameraRef, SUCameraGetAspectRatio)
DEFINE_SETTER(camera_set_aspect_ratio, SUCameraRef, SUCameraSetAspectRatio, NUM2DBL)
DEFINE_DOUBLE_GETTER(camera_height, SUCameraRef, SUCameraGetOrthographicFrustumHeight)
DEFINE_SETTER(camera_set_height, SUCameraRef, SUCameraSetOrthographicFrustumHeight, NUM2DBL)
DEFINE_DOUBLE_GETTER(camera_image_width, SUCameraRef, SUCameraGetImageWidth)
DEFINE_SETTER(camera_set_image_width, SUCameraRef, SUCameraSetImageWidth, NUM2DBL)
DEFINE_BOOL_GETTER(camera_perspective_p, SUCameraRef, SUCameraGetPerspective)
DEFINE_BOOL_SETTER(camera_set_perspective, SUCameraRef, SUCameraSetPerspective)
DEFINE_BOOL_GETTER(camera_fov_is_height_p, SUCameraRef, SUCameraGetFOVIsHeight)
DEFINE_BOOL_GETTER(camera_is_2d_p, SUCameraRef, SUCameraGet2D)
DEFINE_DOUBLE_GETTER(camera_scale_2d, SUCameraRef, SUCameraGetScale2D)

static VALUE camera_center_2d(VALUE self)
{
	struct SUPoint3D center = {0, 0, 0};
	SUCameraGetCenter2D(CAMERA(self), &center);
	return su_point3d_new(center);
}

// Focal length in millimeters, from the field of view and image width.
static VALUE camera_focal_length(VALUE self)
{
	double fov = 0.0;
	double width = 0.0;
	SUCameraGetPerspectiveFrustumFOV(CAMERA(self), &fov);
	SUCameraGetImageWidth(CAMERA(self), &width);
	return DBL2NUM(width / (2.0 * tan(fov * M_PI / 360.0)));
}

static VALUE camera_set_focal_length(VALUE self, VALUE focal_length)
{
	double f = NUM2DBL(focal_length);
	if (f < 1.0 || f > 3000.0)
		rb_raise(rb_eArgError, "focal length must be between 1 and 3000");
	double width = 0.0;
	SUCameraGetImageWidth(CAMERA(self), &width);
	SUCameraSetPerspectiveFrustumFOV(CAMERA(self), 2.0 * atan(width / (2.0 * f)) * 180.0 / M_PI);
	return focal_length;
}

void Init_camera(void)
{
	rb_define_alloc_func(cCamera, camera_alloc);
	rb_define_method(cCamera, "initialize", camera_initialize, -1);
	rb_define_method(cCamera, "set", camera_set, 3);
	rb_define_method(cCamera, "eye", camera_eye, 0);
	rb_define_method(cCamera, "target", camera_target, 0);
	rb_define_method(cCamera, "up", camera_up, 0);
	rb_define_method(cCamera, "direction", camera_direction, 0);
	rb_define_method(cCamera, "xaxis", camera_xaxis, 0);
	rb_define_method(cCamera, "yaxis", camera_yaxis, 0);
	rb_define_method(cCamera, "zaxis", camera_zaxis, 0);
	rb_define_method(cCamera, "description", camera_description, 0);
	rb_define_method(cCamera, "description=", camera_set_description, 1);
	rb_define_method(cCamera, "fov", camera_fov, 0);
	rb_define_method(cCamera, "fov=", camera_set_fov, 1);
	rb_define_method(cCamera, "aspect_ratio", camera_aspect_ratio, 0);
	rb_define_method(cCamera, "aspect_ratio=", camera_set_aspect_ratio, 1);
	rb_define_method(cCamera, "height", camera_height, 0);
	rb_define_method(cCamera, "height=", camera_set_height, 1);
	rb_define_method(cCamera, "image_width", camera_image_width, 0);
	rb_define_method(cCamera, "image_width=", camera_set_image_width, 1);
	rb_define_method(cCamera, "perspective?", camera_perspective_p, 0);
	rb_define_method(cCamera, "perspective=", camera_set_perspective, 1);
	rb_define_method(cCamera, "fov_is_height?", camera_fov_is_height_p, 0);
	rb_define_method(cCamera, "is_2d?", camera_is_2d_p, 0);
	rb_define_method(cCamera, "scale_2d", camera_scale_2d, 0);
	rb_define_method(cCamera, "center_2d", camera_center_2d, 0);
	rb_define_method(cCamera, "focal_length", camera_focal_length, 0);
	rb_define_method(cCamera, "focal_length=", camera_set_focal_length, 1);
}
