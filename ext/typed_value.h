#ifndef SCRIPTUP_TYPED_VALUE_H
#define SCRIPTUP_TYPED_VALUE_H

#include <ruby.h>
#include <SketchUpAPI/sketchup.h>

VALUE TypedValue_get_value_type(SUTypedValueRef* typedValue);
VALUE TypedValue_set_value_type(SUTypedValueRef* typedValue, VALUE value);


#endif // SCRIPTUP_TYPED_VALUE_H