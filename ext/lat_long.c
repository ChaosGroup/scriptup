#include <scriptup.h>

struct lat_long
{
	double latitude;
	double longitude;
};

#define LL(obj) ((struct lat_long*)DATA_PTR(obj))

static VALUE latlong_alloc(VALUE klass)
{
	struct lat_long* ll;
	VALUE obj = Data_Make_Struct(klass, struct lat_long, 0, RUBY_DEFAULT_FREE, ll);
	*ll = (struct lat_long){0.0, 0.0};
	return obj;
}

VALUE su_lat_long_new(double latitude, double longitude)
{
	VALUE obj = latlong_alloc(cLatLong);
	*LL(obj) = (struct lat_long){latitude, longitude};
	return obj;
}

static void latlong_set(VALUE self, VALUE lat, VALUE lng)
{
	double latitude = NUM2DBL(lat);
	double longitude = NUM2DBL(lng);
	if (latitude < -90.0 || latitude > 90.0)
		rb_raise(rb_eArgError, "latitude must be between -90 and 90");
	if (longitude < -180.0 || longitude > 180.0)
		rb_raise(rb_eArgError, "longitude must be between -180 and 180");
	*LL(self) = (struct lat_long){latitude, longitude};
}

static VALUE latlong_initialize(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 0, 2);
	if (argc == 2)
		latlong_set(self, argv[0], argv[1]);
	else if (argc == 1 && rb_obj_is_kind_of(argv[0], cLatLong))
		*LL(self) = *LL(argv[0]);
	else if (argc == 1)
	{
		double xy[2];
		su_array_to_xy(argv[0], xy);
		latlong_set(self, DBL2NUM(xy[0]), DBL2NUM(xy[1]));
	}
	return self;
}

static VALUE latlong_initialize_copy(VALUE self, VALUE other)
{
	*LL(self) = *LL(other);
	return self;
}

static VALUE latlong_latitude(VALUE self)
{
	return DBL2NUM(LL(self)->latitude);
}

static VALUE latlong_longitude(VALUE self)
{
	return DBL2NUM(LL(self)->longitude);
}

static VALUE latlong_to_a(VALUE self)
{
	return rb_ary_new_from_args(2, latlong_latitude(self), latlong_longitude(self));
}

static VALUE latlong_to_s(VALUE self)
{
	return rb_sprintf("(%"PRIsVALUE", %"PRIsVALUE")", su_format_number(LL(self)->latitude), su_format_number(LL(self)->longitude));
}

void Init_lat_long(void)
{
	rb_define_alloc_func(cLatLong, latlong_alloc);
	rb_define_method(cLatLong, "initialize", latlong_initialize, -1);
	rb_define_method(cLatLong, "initialize_copy", latlong_initialize_copy, 1);
	rb_define_method(cLatLong, "latitude", latlong_latitude, 0);
	rb_define_method(cLatLong, "longitude", latlong_longitude, 0);
	rb_define_method(cLatLong, "to_a", latlong_to_a, 0);
	rb_define_method(cLatLong, "to_s", latlong_to_s, 0);
}
