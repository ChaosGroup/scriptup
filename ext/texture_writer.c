#include <scriptup.h>

#define WRITER(self) SU_REF(SUTextureWriterRef, self)

enum { FILE_WRITE_OK = 0, FILE_WRITE_FAILED_INVALID_TYPE = 1, FILE_WRITE_FAILED_UNKNOWN = 2 };

static void texturewriter_free(void* ptr)
{
	SUTextureWriterRef writer = {ptr};
	if (ptr)
		SUTextureWriterRelease(&writer);
}

// Sketchup.create_texture_writer
static VALUE sketchup_create_texture_writer(VALUE self)
{
	SUTextureWriterRef writer = SU_INVALID;
	SUTextureWriterCreate(&writer);
	return Data_Wrap_Struct(cTextureWriter, 0, texturewriter_free, writer.ptr);
}

static VALUE texturewriter_count(VALUE self)
{
	size_t count = 0;
	SUTextureWriterGetNumTextures(WRITER(self), &count);
	return SIZET2NUM(count);
}

static VALUE texturewriter_filename(VALUE self, VALUE handle)
{
	SUStringRef path = SU_INVALID;
	SUStringCreate(&path);
	enum SUResult result = SUTextureWriterGetTextureFilePath(WRITER(self), NUM2LONG(handle), &path);
	VALUE output = su_string_to_rb(path);
	SUStringRelease(&path);
	if (result != SU_ERROR_NONE)
		rb_raise(rb_eArgError, "invalid texture handle");
	return output;
}

static VALUE texturewriter_load(int argc, VALUE* argv, VALUE self)
{
	VALUE entity, side;
	rb_scan_args(argc, argv, "11", &entity, &side);
	su_unwrap(entity, cEntity);
	long id = 0;
	if (rb_obj_is_kind_of(entity, cFace))
	{
		long front = 0, back = 0;
		SUTextureWriterLoadFace(WRITER(self), SU_REF(SUFaceRef, entity), &front, &back);
		id = NIL_P(side) || RTEST(side) ? front : back;
	}
	else
		SUTextureWriterLoadEntity(WRITER(self), SU_REF(SUEntityRef, entity), &id);
	return LONG2NUM(id);
}

static VALUE texturewriter_handle(int argc, VALUE* argv, VALUE self)
{
	VALUE entity, side;
	rb_scan_args(argc, argv, "11", &entity, &side);
	su_unwrap(entity, cEntity);
	long id = 0;
	if (rb_obj_is_kind_of(entity, cFace))
		SUTextureWriterGetTextureIdForFace(WRITER(self), SU_REF(SUFaceRef, entity), NIL_P(side) || RTEST(side), &id);
	else
		SUTextureWriterGetTextureIdForEntity(WRITER(self), SU_REF(SUEntityRef, entity), &id);
	return LONG2NUM(id);
}

static VALUE texturewriter_write(int argc, VALUE* argv, VALUE self)
{
	rb_check_arity(argc, 2, 3);
	VALUE path = argv[argc - 1];
	VALUE handle = argc == 3 ? texturewriter_handle(2, argv, self) : texturewriter_handle(1, argv, self);
	if (NUM2LONG(handle) == 0)
		return INT2FIX(FILE_WRITE_FAILED_UNKNOWN);
	enum SUResult result = SUTextureWriterWriteTexture(WRITER(self), NUM2LONG(handle), StringValueCStr(path), false);
	if (result == SU_ERROR_SERIALIZATION)
		return INT2FIX(FILE_WRITE_FAILED_INVALID_TYPE);
	return INT2FIX(result == SU_ERROR_NONE ? FILE_WRITE_OK : FILE_WRITE_FAILED_UNKNOWN);
}

static VALUE texturewriter_write_all(VALUE self, VALUE directory, VALUE filename_format)
{
	enum SUResult result = SUTextureWriterWriteAllTextures(WRITER(self), StringValueCStr(directory));
	return INT2FIX(result == SU_ERROR_NONE ? FILE_WRITE_OK : FILE_WRITE_FAILED_UNKNOWN);
}

void Init_texture_writer(void)
{
	rb_define_const(rb_cObject, "FILE_WRITE_OK", INT2FIX(FILE_WRITE_OK));
	rb_define_const(rb_cObject, "FILE_WRITE_FAILED_INVALID_TYPE", INT2FIX(FILE_WRITE_FAILED_INVALID_TYPE));
	rb_define_const(rb_cObject, "FILE_WRITE_FAILED_UNKNOWN", INT2FIX(FILE_WRITE_FAILED_UNKNOWN));
	rb_define_singleton_method(mSketchup, "create_texture_writer", sketchup_create_texture_writer, 0);
	rb_undef_method(rb_singleton_class(cTextureWriter), "new");
	rb_define_method(cTextureWriter, "count", texturewriter_count, 0);
	rb_define_method(cTextureWriter, "length", texturewriter_count, 0);
	rb_define_method(cTextureWriter, "filename", texturewriter_filename, 1);
	rb_define_method(cTextureWriter, "load", texturewriter_load, -1);
	rb_define_method(cTextureWriter, "handle", texturewriter_handle, -1);
	rb_define_method(cTextureWriter, "write", texturewriter_write, -1);
	rb_define_method(cTextureWriter, "write_all", texturewriter_write_all, 2);
}
