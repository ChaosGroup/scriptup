#include <scriptup.h>

VALUE su_wrap(VALUE klass, void* ptr)
{
	return Data_Wrap_Struct(klass, 0, 0, ptr);
}

VALUE su_wrap_or_nil(VALUE klass, void* ptr)
{
	return ptr ? su_wrap(klass, ptr) : Qnil;
}

void* su_unwrap(VALUE obj, VALUE klass)
{
	if (!rb_obj_is_kind_of(obj, klass))
		rb_raise(rb_eTypeError, "wrong argument type %"PRIsVALUE" (expected %"PRIsVALUE")", rb_obj_class(obj), klass);
	return DATA_PTR(obj);
}

VALUE su_string_to_rb(SUStringRef string)
{
	size_t length = 0;
	SUStringGetUTF8Length(string, &length);
	VALUE output = rb_utf8_str_new(0, (long)length);
	SUStringGetUTF8(string, length + 1, RSTRING_PTR(output), &length);
	return output;
}

int su_clamp_int(VALUE value, int min, int max)
{
	int i = NUM2INT(value);
	return i < min ? min : i > max ? max : i;
}

double su_clamp_double(VALUE value, double min, double max)
{
	double d = NUM2DBL(value);
	return d < min ? min : d > max ? max : d;
}

VALUE su_yield_each(VALUE self, VALUE ary)
{
	for (long i = 0; i < RARRAY_LEN(ary); i++)
		rb_yield(RARRAY_AREF(ary, i));
	return self;
}

VALUE su_find_by(VALUE ary, const char* method, VALUE key)
{
	ID id = (rb_intern)(method);
	for (long i = 0; i < RARRAY_LEN(ary); i++)
	{
		VALUE item = RARRAY_AREF(ary, i);
		if (rb_equal(rb_funcall(item, id, 0), key))
			return item;
	}
	return Qnil;
}

VALUE su_fetch(VALUE ary, VALUE index)
{
	long i = NUM2LONG(index);
	if (i < 0 || i >= RARRAY_LEN(ary))
		rb_raise(rb_eIndexError, "index %ld out of range [0, %ld)", i, RARRAY_LEN(ary));
	return RARRAY_AREF(ary, i);
}

VALUE su_ruby_from_typed_value(SUTypedValueRef value)
{
	enum SUTypedValueType type = SUTypedValueType_Empty;
	if (SUTypedValueGetType(value, &type) != SU_ERROR_NONE)
		return Qnil;
	switch (type)
	{
		case SUTypedValueType_Byte:
		{
			char byte = 0;
			SUTypedValueGetByte(value, &byte);
			return INT2FIX(byte);
		}
		case SUTypedValueType_Short:
		{
			int16_t i = 0;
			SUTypedValueGetInt16(value, &i);
			return INT2FIX(i);
		}
		case SUTypedValueType_Int32:
		{
			int32_t i = 0;
			SUTypedValueGetInt32(value, &i);
			return INT2NUM(i);
		}
		case SUTypedValueType_Float:
		{
			float f = 0;
			SUTypedValueGetFloat(value, &f);
			return DBL2NUM(f);
		}
		case SUTypedValueType_Double:
		{
			double d = 0;
			SUTypedValueGetDouble(value, &d);
			return DBL2NUM(d);
		}
		case SUTypedValueType_Bool:
		{
			bool b = false;
			SUTypedValueGetBool(value, &b);
			return BOOL2RB(b);
		}
		case SUTypedValueType_String:
		{
			SUStringRef string = SU_INVALID;
			SUStringCreate(&string);
			SUTypedValueGetString(value, &string);
			VALUE output = su_string_to_rb(string);
			SUStringRelease(&string);
			return output;
		}
		case SUTypedValueType_Color:
		{
			SUColor color = {0, 0, 0, 255};
			SUTypedValueGetColor(value, &color);
			return su_color_new(color);
		}
		case SUTypedValueType_Time:
		{
			int64_t time = 0;
			SUTypedValueGetTime(value, &time);
			return rb_time_new((time_t)time, 0);
		}
		case SUTypedValueType_Vector3D:
		{
			double v[3] = {0, 0, 0};
			SUTypedValueGetVector3d(value, v);
			return su_vector3d_new((struct SUVector3D){v[0], v[1], v[2]});
		}
		case SUTypedValueType_Array:
		{
			size_t count = 0;
			SUTypedValueGetNumArrayItems(value, &count);
			VALUE ary = rb_ary_new_capa((long)count);
			if (count == 0)
				return ary;
			// The items stay owned by the array.
			SUTypedValueRef* items = ZALLOC_N(SUTypedValueRef, count);
			SUTypedValueGetArrayItems(value, count, items, &count);
			for (size_t i = 0; i < count; i++)
				rb_ary_push(ary, su_ruby_from_typed_value(items[i]));
			xfree(items);
			return ary;
		}
		case SUTypedValueType_Empty:
		default:
			return Qnil;
	}
}

void su_typed_value_from_ruby(SUTypedValueRef value, VALUE object)
{
	switch (rb_type(object))
	{
		case T_FLOAT:
			SUTypedValueSetDouble(value, NUM2DBL(object));
			break;
		case T_FIXNUM:
		case T_BIGNUM:
			SUTypedValueSetInt32(value, NUM2INT(object));
			break;
		case T_TRUE:
		case T_FALSE:
			SUTypedValueSetBool(value, RTEST(object));
			break;
		case T_STRING:
			SUTypedValueSetString(value, StringValueCStr(object));
			break;
		case T_ARRAY:
		{
			long count = RARRAY_LEN(object);
			SUTypedValueRef* items = ZALLOC_N(SUTypedValueRef, count > 0 ? count : 1);
			for (long i = 0; i < count; i++)
			{
				items[i].ptr = NULL;
				SUTypedValueCreate(&items[i]);
				su_typed_value_from_ruby(items[i], RARRAY_AREF(object, i));
			}
			SUTypedValueSetArrayItems(value, (size_t)count, items);
			for (long i = 0; i < count; i++)
				SUTypedValueRelease(&items[i]);
			xfree(items);
			break;
		}
		default:
			if (rb_obj_is_kind_of(object, cColor))
			{
				SUTypedValueSetColor(value, DATA_PTR(object));
				break;
			}
			if (rb_obj_is_kind_of(object, cVector3d))
			{
				struct SUVector3D v = su_to_vector3d(object);
				double xyz[3] = {v.x, v.y, v.z};
				SUTypedValueSetVector3d(value, xyz);
				break;
			}
			if (rb_obj_is_kind_of(object, rb_cTime))
			{
				SUTypedValueSetTime(value, NUM2LL(rb_funcall(object, (rb_intern)("to_i"), 0)));
				break;
			}
			rb_raise(rb_eNotImpError, "unsupported attribute value type %"PRIsVALUE, rb_obj_class(object));
	}
}

// Accepts a 2 or 3 element numeric array; z defaults to 0.
void su_array_to_xyz(VALUE ary, double xyz[3])
{
	if (!RB_TYPE_P(ary, T_ARRAY))
		rb_raise(rb_eTypeError, "wrong argument type %"PRIsVALUE" (expected Array)", rb_obj_class(ary));
	long length = RARRAY_LEN(ary);
	if (length != 2 && length != 3)
		rb_raise(rb_eArgError, "expected an array of 2 or 3 numbers, got %ld", length);
	xyz[0] = NUM2DBL(RARRAY_AREF(ary, 0));
	xyz[1] = NUM2DBL(RARRAY_AREF(ary, 1));
	xyz[2] = length == 3 ? NUM2DBL(RARRAY_AREF(ary, 2)) : 0.0;
}

// Formats like SketchUp: whole numbers without a decimal part.
VALUE su_format_number(double value)
{
	if (value == floor(value) && fabs(value) < 1e15)
		return rb_sprintf("%lld", (long long)value);
	return rb_funcall(DBL2NUM(value), (rb_intern)("to_s"), 0);
}

void su_array_to_xy(VALUE ary, double xy[2])
{
	if (!RB_TYPE_P(ary, T_ARRAY))
		rb_raise(rb_eTypeError, "wrong argument type %"PRIsVALUE" (expected Array)", rb_obj_class(ary));
	if (RARRAY_LEN(ary) != 2)
		rb_raise(rb_eArgError, "expected an array of 2 numbers, got %ld", RARRAY_LEN(ary));
	xy[0] = NUM2DBL(RARRAY_AREF(ary, 0));
	xy[1] = NUM2DBL(RARRAY_AREF(ary, 1));
}
