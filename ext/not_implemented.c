#include <stdbool.h>
#include <not_implemented.h>
#include <utils.h>
#include <SketchUpAPI/sketchup.h>

VALUE BoundingBox_Init(VALUE Sketchup, VALUE Sketchup_Object)
{
	VALUE Sketchup_BoundingBox = rb_define_class_under(Sketchup, BOUNDINGBOX, Sketchup_Object);
	rb_undef_alloc_func(Sketchup_BoundingBox);
	return Sketchup_BoundingBox;
}

VALUE Face_Init(VALUE Sketchup, VALUE DrawingElement)
{
	VALUE Sketchup_Face = rb_define_class_under(Sketchup, FACE, DrawingElement);
	rb_undef_alloc_func(Sketchup_Face);
	return Sketchup_Face;
}

VALUE Layer_Init(VALUE Sketchup, VALUE Entity)
{
	VALUE Sketchup_Layer = rb_define_class_under(Sketchup, LAYER, Entity);
	rb_undef_alloc_func(Sketchup_Layer);
	return Sketchup_Layer;
}

VALUE ConstructionLine_Init(VALUE Sketchup, VALUE DrawingElement)
{
	VALUE Sketchup_ConstructionLine = rb_define_class_under(Sketchup, CONSTRUCTIONLINE, DrawingElement);
	rb_undef_alloc_func(Sketchup_ConstructionLine);
	return Sketchup_ConstructionLine;
}

VALUE ConstructionPoint_Init(VALUE Sketchup, VALUE DrawingElement)
{
	VALUE Sketchup_ConstructionPoint = rb_define_class_under(Sketchup, CONSTRUCTIONPOINT, DrawingElement);
	rb_undef_alloc_func(Sketchup_ConstructionPoint);
	return Sketchup_ConstructionPoint;
}

VALUE Dimension_Init(VALUE Sketchup, VALUE DrawingElement)
{
	VALUE Sketchup_Dimension = rb_define_class_under(Sketchup, DIMENSION, DrawingElement);
	rb_undef_alloc_func(Sketchup_Dimension);
	return Sketchup_Dimension;
}

VALUE Edge_Init(VALUE Sketchup, VALUE DrawingElement)
{
	VALUE Sketchup_Edge = rb_define_class_under(Sketchup, EDGE, DrawingElement);
	rb_undef_alloc_func(Sketchup_Edge);
	return Sketchup_Edge;
}

VALUE Group_Init(VALUE Sketchup, VALUE DrawingElement)
{
	VALUE Sketchup_Group = rb_define_class_under(Sketchup, GROUP, DrawingElement);
	rb_undef_alloc_func(Sketchup_Group);
	return Sketchup_Group;
}

VALUE Image_Init(VALUE Sketchup, VALUE DrawingElement)
{
	VALUE Sketchup_Image = rb_define_class_under(Sketchup, IMAGE, DrawingElement);
	rb_undef_alloc_func(Sketchup_Image);
	return Sketchup_Image;
}

VALUE SectionPlane_Init(VALUE Sketchup, VALUE DrawingElement)
{
	VALUE Sketchup_SectionPlane = rb_define_class_under(Sketchup, SECTIONPLANE, DrawingElement);
	rb_undef_alloc_func(Sketchup_SectionPlane);
	return Sketchup_SectionPlane;
}

VALUE Text_Init(VALUE Sketchup, VALUE DrawingElement)
{
	VALUE Sketchup_Text = rb_define_class_under(Sketchup, TEXT, DrawingElement);
	rb_undef_alloc_func(Sketchup_Text);
	return Sketchup_Text;
}