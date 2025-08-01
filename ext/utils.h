#ifndef SCRIPTUP_UTILS_H
#define SCRIPTUP_UTILS_H

#include <ruby.h>

#ifdef TEXT
#undef TEXT
#endif //TEXT
#define SKETCHUP "Sketchup"
#define GEOM "Geom"
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
#define BOUNDINGBOX "BoundingBox"
#define FACE "Face"
#define LAYER "Layer"
#define CONSTRUCTIONLINE "ConstructionLine"
#define CONSTRUCTIONPOINT "ConstructionPoint"
#define DIMENSION "Dimension"
#define EDGE "Edge"
#define IMAGE "Image"
#define SECTIONPLANE "SectionPlane"
#define GROUP "Group"
#define TEXT "Text"
#define IMAGEREP "ImageRep"
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
#define GEOM_BOUNDINGBOX GEOM "::" BOUNDINGBOX
#define SKETCHUP_FACE SKETCHUP "::" FACE
#define SKETCHUP_LAYER SKETCHUP "::" LAYER
#define SKETCHUP_CONSTRUCTIONLINE SKETCHUP "::" CONSTRUCTIONLINE
#define SKETCHUP_CONSTRUCTIONPOINT SKETCHUP "::" CONSTRUCTIONPOINT
#define SKETCHUP_DIMENSION SKETCHUP "::" DIMENSION
#define SKETCHUP_EDGER SKETCHUP "::" EDGE
#define SKETCHUP_IMAGE SKETCHUP "::" IMAGE
#define SKETCHUP_SECTIONPLANE SKETCHUP "::" SECTIONPLANE
#define SKETCHUP_GROUP SKETCHUP "::" GROUP
#define SKETCHUP_TEXT SKETCHUP "::" TEXT
#define SKETCHUP_IMAGEREP SKETCHUP "::" IMAGEREP

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wbackslash-newline-escape"

#define RDEF(arg) (arg != Qundef)

#define FOREACH(SUGetNum, SUGet, SURef, ref, callback, userdata) {\
	size_t num; \
	SUGetNum(ref, &num); \
	SURef* array = malloc(num * sizeof(SURef)); \
	SUGet(ref, num, array, &num); \
	for(size_t i = 0; i < num; i++) \
		callback(array[i], userdata); \
	free(array); \
}

#define FOREACHSTRING(SUGetNum, SUGet, SURef, ref, callback, userdata) {\
	size_t num; \
	SUGetNum(ref, &num); \
	SURef* array = malloc(num * sizeof(SURef)); \
	for (size_t i = 0; i < num; i++) \
	{ \
		array[i].ptr = 0; \
		SUStringCreate(&array[i]); \
	} \
	SUGet(ref, num, array, &num); \
	for (size_t i = 0; i < num; i++) \
		callback(array[i], userdata); \
	for (size_t i = 0; i < num; i++) \
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

#pragma clang diagnostic pop

#endif // SCRIPTUP_UTILS_H