#include <scriptup.h>

#define LENGTH_TOLERANCE 1.0e-3

#define LENGTH(obj) (*(double*)DATA_PTR(obj))

VALUE su_length_new(double inches)
{
	double* value;
	VALUE obj = Data_Make_Struct(cLength, double, 0, RUBY_DEFAULT_FREE, value);
	*value = inches;
	return obj;
}

static SULengthFormatterRef length_formatter(void)
{
	SULengthFormatterRef formatter = SU_INVALID;
	SULengthFormatterCreate(&formatter);
	SUModelRef model = Sketchup_get_active_model();
	if (SUIsValid(model))
		SUModelGetLengthFormatter(model, &formatter);
	return formatter;
}

static VALUE length_to_f(VALUE self)
{
	return DBL2NUM(LENGTH(self));
}

static VALUE length_to_l(VALUE self)
{
	return self;
}

static VALUE length_to_s(VALUE self)
{
	SULengthFormatterRef formatter = length_formatter();
	SUStringRef string = SU_INVALID;
	SUStringCreate(&string);
	SULengthFormatterGetLengthString(formatter, LENGTH(self), false, &string);
	VALUE output = su_string_to_rb(string);
	SUStringRelease(&string);
	SULengthFormatterRelease(&formatter);
	return output;
}

static VALUE length_inspect(VALUE self)
{
	return rb_funcall(length_to_f(self), (rb_intern)("inspect"), 0);
}

static int length_compare(VALUE self, VALUE other)
{
	double diff = LENGTH(self) - NUM2DBL(other);
	return diff > LENGTH_TOLERANCE ? 1 : diff < -LENGTH_TOLERANCE ? -1 : 0;
}

static VALUE length_cmp(VALUE self, VALUE other)
{
	if (!rb_obj_is_kind_of(other, rb_cNumeric))
		return Qnil;
	return INT2FIX(length_compare(self, other));
}

static VALUE length_eq(VALUE self, VALUE other)
{
	return BOOL2RB(rb_obj_is_kind_of(other, rb_cNumeric) && length_compare(self, other) == 0);
}

static VALUE length_lt(VALUE self, VALUE other) { return BOOL2RB(length_compare(self, other) < 0); }
static VALUE length_le(VALUE self, VALUE other) { return BOOL2RB(length_compare(self, other) <= 0); }
static VALUE length_gt(VALUE self, VALUE other) { return BOOL2RB(length_compare(self, other) > 0); }
static VALUE length_ge(VALUE self, VALUE other) { return BOOL2RB(length_compare(self, other) >= 0); }

static VALUE length_hash(VALUE self)
{
	return rb_funcall(length_to_f(self), (rb_intern)("hash"), 0);
}

static VALUE length_eql_p(VALUE self, VALUE other)
{
	return BOOL2RB(rb_obj_is_kind_of(other, cLength) && LENGTH(self) == LENGTH(other));
}

// Arithmetic behaves like Float and returns Float.
static VALUE length_coerce(VALUE self, VALUE other)
{
	return rb_assoc_new(DBL2NUM(NUM2DBL(other)), length_to_f(self));
}

#define DEFINE_FLOAT_OP(fn, op) \
	static VALUE fn(VALUE self, VALUE other) \
	{ \
		return rb_funcall(length_to_f(self), (rb_intern)(op), 1, other); \
	}
DEFINE_FLOAT_OP(length_plus, "+")
DEFINE_FLOAT_OP(length_minus, "-")
DEFINE_FLOAT_OP(length_mul, "*")
DEFINE_FLOAT_OP(length_div, "/")

static VALUE length_uminus(VALUE self)
{
	return DBL2NUM(-LENGTH(self));
}

static VALUE length_to_i(VALUE self)
{
	return rb_funcall(length_to_f(self), (rb_intern)("to_i"), 0);
}

// Numeric and String extensions. Lengths are stored in inches.
#define DEFINE_UNIT(name, inches_per_unit) \
	static VALUE numeric_##name(VALUE self) { return su_length_new(NUM2DBL(self) * (inches_per_unit)); } \
	static VALUE numeric_to_##name(VALUE self) { return DBL2NUM(NUM2DBL(self) / (inches_per_unit)); }
DEFINE_UNIT(inch, 1.0)
DEFINE_UNIT(feet, 12.0)
DEFINE_UNIT(yard, 36.0)
DEFINE_UNIT(mile, 63360.0)
DEFINE_UNIT(mm, 1.0 / 25.4)
DEFINE_UNIT(cm, 1.0 / 2.54)
DEFINE_UNIT(m, 100.0 / 2.54)
DEFINE_UNIT(km, 100000.0 / 2.54)

static VALUE numeric_to_l(VALUE self)
{
	return su_length_new(NUM2DBL(self));
}

static VALUE numeric_degrees(VALUE self)
{
	return DBL2NUM(NUM2DBL(self) * M_PI / 180.0);
}

static VALUE numeric_radians(VALUE self)
{
	return DBL2NUM(NUM2DBL(self) * 180.0 / M_PI);
}

static VALUE string_to_l(VALUE self)
{
	SULengthFormatterRef formatter = length_formatter();
	SUStringRef string = SU_INVALID;
	SUStringCreateFromUTF8(&string, StringValueCStr(self));
	double value = 0.0;
	enum SUResult result = SULengthFormatterParseString(formatter, string, &value);
	SUStringRelease(&string);
	SULengthFormatterRelease(&formatter);
	if (result != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "Cannot convert \"%"PRIsVALUE"\" to Length", self);
	return su_length_new(value);
}

typedef enum SUResult (*format_fn)(SULengthFormatterRef, double, SUStringRef*);

static enum SUResult format_length_string(SULengthFormatterRef f, double v, SUStringRef* s) { return SULengthFormatterGetLengthString(f, v, false, s); }

static VALUE format_with(double value, format_fn fn, VALUE precision)
{
	SULengthFormatterRef formatter = length_formatter();
	if (!NIL_P(precision))
		SULengthFormatterSetPrecision(formatter, NUM2SIZET(precision));
	SUStringRef string = SU_INVALID;
	SUStringCreate(&string);
	fn(formatter, value, &string);
	VALUE output = su_string_to_rb(string);
	SUStringRelease(&string);
	SULengthFormatterRelease(&formatter);
	return output;
}

static VALUE sketchup_format_length(int argc, VALUE* argv, VALUE self)
{
	VALUE number, precision;
	rb_scan_args(argc, argv, "11", &number, &precision);
	return format_with(NUM2DBL(number), format_length_string, precision);
}

static VALUE sketchup_format_area(VALUE self, VALUE number)
{
	return format_with(NUM2DBL(number), SULengthFormatterGetAreaString, Qnil);
}

static VALUE sketchup_format_volume(VALUE self, VALUE number)
{
	return format_with(NUM2DBL(number), SULengthFormatterGetVolumeString, Qnil);
}

static VALUE sketchup_parse_length(VALUE self, VALUE string)
{
	SULengthFormatterRef formatter = length_formatter();
	SUStringRef input = SU_INVALID;
	SUStringCreateFromUTF8(&input, StringValueCStr(string));
	double value = 0.0;
	enum SUResult result = SULengthFormatterParseString(formatter, input, &value);
	SUStringRelease(&input);
	SULengthFormatterRelease(&formatter);
	return result == SU_ERROR_NONE ? DBL2NUM(value) : Qnil;
}

void Init_length(void)
{
	rb_define_singleton_method(mSketchup, "format_length", sketchup_format_length, -1);
	rb_define_singleton_method(mSketchup, "format_area", sketchup_format_area, 1);
	rb_define_singleton_method(mSketchup, "format_volume", sketchup_format_volume, 1);
	rb_define_singleton_method(mSketchup, "parse_length", sketchup_parse_length, 1);
	static const struct { const char* name; int value; } constants[] = {
		{"Architectural", 0}, {"Decimal", 1}, {"Engineering", 2}, {"Fractional", 3},
		{"Inches", 0}, {"Feet", 1}, {"Millimeter", 2}, {"Centimeter", 3}, {"Meter", 4}, {"Yard", 5},
		{"SquareInches", 0}, {"SquareFeet", 1}, {"SquareMillimeter", 2}, {"SquareCentimeter", 3},
		{"SquareMeter", 4}, {"SquareYard", 5},
		{"CubicInches", 0}, {"CubicFeet", 1}, {"CubicMillimeter", 2}, {"CubicCentimeter", 3},
		{"CubicMeter", 4}, {"CubicYard", 5}, {"Liter", 6}, {"USGallon", 7},
	};
	for (size_t i = 0; i < sizeof(constants) / sizeof(constants[0]); i++)
		rb_define_const(cLength, constants[i].name, INT2FIX(constants[i].value));

	rb_undef_method(rb_singleton_class(cLength), "new");
	rb_define_method(cLength, "to_f", length_to_f, 0);
	rb_define_method(cLength, "to_l", length_to_l, 0);
	rb_define_method(cLength, "to_s", length_to_s, 0);
	rb_define_method(cLength, "inspect", length_inspect, 0);
	rb_define_method(cLength, "<=>", length_cmp, 1);
	rb_define_method(cLength, "==", length_eq, 1);
	rb_define_method(cLength, "<", length_lt, 1);
	rb_define_method(cLength, "<=", length_le, 1);
	rb_define_method(cLength, ">", length_gt, 1);
	rb_define_method(cLength, ">=", length_ge, 1);
	rb_define_method(cLength, "hash", length_hash, 0);
	rb_define_method(cLength, "eql?", length_eql_p, 1);
	rb_define_method(cLength, "coerce", length_coerce, 1);
	rb_define_method(cLength, "+", length_plus, 1);
	rb_define_method(cLength, "-", length_minus, 1);
	rb_define_method(cLength, "*", length_mul, 1);
	rb_define_method(cLength, "/", length_div, 1);
	rb_define_method(cLength, "-@", length_uminus, 0);
	rb_define_method(cLength, "to_i", length_to_i, 0);

#define DEFINE_UNIT_METHODS(name) \
	rb_define_method(rb_cNumeric, #name, numeric_##name, 0); \
	rb_define_method(rb_cNumeric, "to_" #name, numeric_to_##name, 0);
	DEFINE_UNIT_METHODS(inch)
	DEFINE_UNIT_METHODS(feet)
	DEFINE_UNIT_METHODS(yard)
	DEFINE_UNIT_METHODS(mile)
	DEFINE_UNIT_METHODS(mm)
	DEFINE_UNIT_METHODS(cm)
	DEFINE_UNIT_METHODS(m)
	DEFINE_UNIT_METHODS(km)
	rb_define_method(rb_cNumeric, "to_l", numeric_to_l, 0);
	rb_define_method(rb_cNumeric, "degrees", numeric_degrees, 0);
	rb_define_method(rb_cNumeric, "radians", numeric_radians, 0);
	rb_define_method(rb_cString, "to_l", string_to_l, 0);
}
