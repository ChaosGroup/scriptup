#include <stdbool.h>
#include <typed_value.h>
#include <utils.h>

VALUE TypedValue_get_value_type(SUTypedValueRef* typedValue)
{
	enum SUTypedValueType type = SUTypedValueType_Empty;
	enum SUResult result = SUTypedValueGetType(*typedValue, &type);
	if(result != SU_ERROR_NONE)
		return Qnil;
	switch (type)
	{
		case SUTypedValueType_Empty:
		{
			return Qnil;
		}
		case SUTypedValueType_Byte:
		{
			return Qnil;
		}
		case SUTypedValueType_Short:
		{
			int16_t value;
			SUTypedValueGetInt16(*typedValue, &value);
			return INT2FIX(value);
		}
		case SUTypedValueType_Int32:
		{
			int32_t value;
			SUTypedValueGetInt32(*typedValue, &value);
			return INT2NUM(value);
		}
		case SUTypedValueType_Float:
		{
			float value;
			SUTypedValueGetFloat(*typedValue, &value);
			return DBL2NUM(value);
		}
		case SUTypedValueType_Double:
		{
			double value;
			SUTypedValueGetDouble(*typedValue, &value);
			return DBL2NUM(value);
		}
 		case SUTypedValueType_Bool:
		{
			bool value;
			SUTypedValueGetBool(*typedValue, &value);
			return value ? Qtrue : Qfalse;
		}
		case SUTypedValueType_String:
		{
			VALUE output;
			GETSTRING(SUTypedValueGetString, *typedValue, output);
			return output;
		} 
		case SUTypedValueType_Color:
		case SUTypedValueType_Time:
		case SUTypedValueType_Vector3D:
		case SUTypedValueType_Array:
		{
			rb_raise(rb_eStandardError, "unimplemented");
		}
	}
}

VALUE TypedValue_set_value_type(SUTypedValueRef* typedValue, VALUE value)
{
	VALUE type = rb_type(value);
	switch (type)
	{
		case RUBY_T_FLOAT:
		{
			SUTypedValueSetFloat(*typedValue, NUM2DBL(value));
			break;
		}
		case RUBY_T_STRING:
		{
			SUTypedValueSetString(*typedValue, StringValuePtr(value));
			break;
		}
	}
	return value;
}