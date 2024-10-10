#include <ruby.h>

#ifndef UTILS_H
#define SKETCHUP "Sketchup"
#define MODEL "Model"
#define MATERIAL "Material"
#define MATERIALS "Materials"
#define TEXTURE "Texture"
#define ENTITY "Entity"
#define ENTITIES "Entities"
#define COLOR "Color"
#define COMPONENTDEFINITION "ComponentDefinition"
#define COMPONENTINSTANCE "ComponentInstance"
#define DEFINITIONLIST "DefinitionList"
#define ATTRIBUTEDICTIONARY "AttributeDictionary"
#define ATTRIBUTEDICTIONARIES "AttributeDictionaries"
#define DRAWINGELEMENT "DrawingElement"
#define BEHAVIOR "Behavior"
#define SKETCHUP_MATERIAL SKETCHUP "::" MATERIAL
#define SKETCHUP_MATERIALS SKETCHUP "::" MATERIALS
#define SKETCHUP_TEXTURE SKETCHUP "::" TEXTURE
#define SKETCHUP_ENTITIES SKETCHUP "::" ENTITIES
#define SKETCHUP_COLOR SKETCHUP "::" COLOR
#define SKETCHUP_COMPONENTDEFINITION SKETCHUP "::" COMPONENTDEFINITION
#define SKETCHUP_COMPONENTINSTANCE SKETCHUP "::" COMPONENTINSTANCE
#define SKETCHUP_MODEL SKETCHUP "::" MODEL
#define SKETCHUP_ATTRIBUTEDICTIONARY SKETCHUP "::" ATTRIBUTEDICTIONARY
#define SKETCHUP_ATTRIBUTEDICTIONARIES SKETCHUP "::" ATTRIBUTEDICTIONARIES
#define SKETCHUP_DEFINITIONLIST SKETCHUP "::" DEFINITIONLIST
#define SKETCHUP_BEHAVIOR SKETCHUP "::" BEHAVIOR

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wbackslash-newline-escape"

#define RDEF(arg) (arg != Qundef)

#define FOREACH(SUGetNum, SUGet, SURef, ref, callback, userdata) {\
	size_t num; \
	SUGetNum(ref, &num); \
	SURef* array = malloc(num * sizeof(SURef)); \
	SUGet(ref, num, array, &num); \
	for( int i = 0; i < num; i++) \
		callback(array[i], userdata); \
	free(array); \
}

#define FOREACHSTRING(SUGetNum, SUGet, SURef, ref, callback, userdata) {\
	size_t num; \
	SUGetNum(ref, &num); \
	SURef* array = malloc(num * sizeof(SURef)); \
	for (int i = 0; i < num; i++) \
	{ \
		array[i].ptr = 0; \
		SUStringCreate(&array[i]); \
	} \
	SUGet(ref, num, array, &num); \
	for (int i = 0; i < num; i++) \
		callback(array[i], userdata); \
	for (int i = 0; i < num; i++) \
		SUStringRelease(&array[i]); \
	free(array); \
}

#define GETUTF8FROMSTRING(suString, output) { \
	size_t count; \
	SUStringGetUTF8Length(suString, &count); \
	output = rb_utf8_str_new(0, count); \
	SUStringGetUTF8(suString, count + 1, RSTRING_PTR(output), &count); \
}

#define GETSTRING(SUName, arg, output) { \
	SUStringRef name = SU_INVALID; \
	SUStringCreate(&name); \
	SUName(arg, &name); \
	GETUTF8FROMSTRING(name, output); \
	SUStringRelease(&name); \
}
#endif // UTILS_H

#pragma clang diagnostic pop