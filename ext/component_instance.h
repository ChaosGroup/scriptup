#ifndef SCRIPTUP_COMPONENT_INSTANCE_H
#define SCRIPTUP_COMPONENT_INSTANCE_H

#include <ruby.h>

VALUE ComponentInstance_Init(VALUE namespace_object, VALUE parent_class);

#endif // SCRIPTUP_COMPONENT_INSTANCE_H