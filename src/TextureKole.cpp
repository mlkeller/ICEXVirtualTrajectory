#include "TextureKole.hpp"
#include <cstring>
#include <assert.h>
#include <common.h>

#define COURTEOUS_BIND(_ID, _GET, _TARG) \
	GLint _save = 0;\
	CHECKED_GL_CALL(glGetIntegerv((_GET), &_save));\
	CHECKED_GL_CALL(glBindTexture((_TARG), (_ID)))

#define COURTEOUS_UNBIND(_TARG) glBindTexture((_TARG), _save);

#define COURTEOUS_BIND_2D(_ID) COURTEOUS_BIND((_ID), GL_TEXTURE_BINDING_2D, GL_TEXTURE_2D)
#define COURTEOUS_UNBIND_2D() COURTEOUS_UNBIND(GL_TEXTURE_2D)

// Should be sufficient as a clearing template even for 64 bit 4 component float textures.
static const GLubyte pixel_sample[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void TextureKole::initParams(){
	glTextureParameteri(ID(), GL_TEXTURE_MIN_FILTER, min_filter);
	glTextureParameteri(ID(), GL_TEXTURE_MAG_FILTER, mag_filter);
	glTextureParameteri(ID(), GL_DEPTH_STENCIL_TEXTURE_MODE, depth_stencil_mode);
	glTextureParameteri(ID(), GL_TEXTURE_WRAP_S, texture_extension_mode[0]);
	glTextureParameteri(ID(), GL_TEXTURE_WRAP_T, texture_extension_mode[1]);
	glTextureParameteri(ID(), GL_TEXTURE_WRAP_R, texture_extension_mode[2]);
}

void TextureKole::copyParametersFrom(const TextureKole& other){
	if (other.texture_extension_mode != nullptr) {
		memcpy(this->texture_extension_mode, other.texture_extension_mode, sizeof(GLenum)*3);
	}
	this->depth_stencil_mode = other.depth_stencil_mode;
	this->min_filter = other.min_filter;
	this->mag_filter = other.mag_filter;
	initParams();
}

SimpleTexture2D::SimpleTexture2D(const Image& image, GLenum internal_format, GLenum format, GLenum type) : TextureKole(){
	makeTextureFromImage(image, internal_format, format, type);
}

/* TODO: More validation */
void SimpleTexture2D::makeTextureFromImage(const Image& image, GLenum internal_format, GLenum format, GLenum type) {
	this->texture_type = type;
	this->internal_format = internal_format;
	this->texture_format = format;
	COURTEOUS_BIND_2D(ID());
	CHECKED_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, image.getWidth(), image.getHeight(), 0, format, type, image.data));
	this->x = image.getWidth();
	this->y = image.getHeight();
	this->z = 1;
	glGenerateMipmap(GL_TEXTURE_2D);
	initParams();
	COURTEOUS_UNBIND_2D();
	ASSERT_NO_GLERR();
}

/* TODO: More validation */
void SimpleTexture2D::copyDataFromImage(const Image& image, int x_offset, int y_offset) {
	COURTEOUS_BIND_2D(ID());
	if (image.getWidth() == x && image.getHeight() == y) {
		CHECKED_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, image.getWidth(), image.getHeight(), 0, texture_format, texture_type, image.data));
	}
	else {
		CHECKED_GL_CALL(glTexSubImage2D(
			GL_TEXTURE_2D, 0, x_offset, y_offset, image.getWidth(),
			image.getHeight(), texture_format, texture_type, image.data
		));
	}
	glGenerateMipmap(GL_TEXTURE_2D);
	COURTEOUS_UNBIND_2D();
	ASSERT_NO_GLERR();
}

Image* SimpleTexture2D::extractImage() const{
	return(extractImage(4));
}

Image* SimpleTexture2D::extractImage(int channels) const{
	size_t component_size = 0;

	switch(this->texture_type){
		case GL_UNSIGNED_BYTE:
			component_size = sizeof(GLubyte);
			break;
		case GL_BYTE:
			component_size = sizeof(GLbyte);
			break;
		case GL_UNSIGNED_SHORT:
			component_size = sizeof(GLushort);
			break;
		case GL_SHORT:
			component_size = sizeof(GLshort);
			break;
		case GL_UNSIGNED_INT:
			component_size = sizeof(GLuint);
			break;
		case GL_INT:
			component_size = sizeof(GLint);
			break;
		case GL_FLOAT:
			component_size = sizeof(GLfloat);
			break;
		default:
			fprintf(stderr, "Error!: Attempting to write unsupported image type!\n");
			exit(EXIT_FAILURE);
			break;
	}

	Image* extracted_image = new Image(x, y, channels, component_size);
	COURTEOUS_BIND_2D(ID());
	CHECKED_GL_CALL(glGetTexImage(GL_TEXTURE_2D, 0, this->texture_format, this->texture_type, extracted_image->data));
	COURTEOUS_UNBIND_2D();
	return(extracted_image); 
}

void SimpleTexture2D::makeEmpty2DTexture(GLuint x, GLuint y, GLenum internal_format, GLenum format, GLenum type, GLenum min_filter, GLenum mag_filter, bool skip_zero_fill)
{
	this->internal_format = internal_format;
	this->texture_format = format;
	this->texture_type = type;
	this->min_filter = min_filter;
	this->mag_filter = mag_filter;
	this->x = x;
	this->y = y;
	this->z = 1;

	COURTEOUS_BIND_2D(ID());
	//CHECKED_GL_CALL(glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, x, y));
	CHECKED_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, x, y, 0, format, type, nullptr));
	if(!skip_zero_fill){
		CHECKED_GL_CALL(glClearTexImage(ID(), 0, format, type, pixel_sample));
	}
	CHECKED_GL_CALL(initParams());
	COURTEOUS_UNBIND_2D();
}

void SimpleTexture2D::fillSolid(const GLubyte* sample){
	COURTEOUS_BIND_2D(ID());
	CHECKED_GL_CALL(glClearTexImage(ID(), 0, texture_format, texture_type, sample != nullptr ? sample : pixel_sample));
	COURTEOUS_UNBIND_2D();
}

void TextureArray::makeEmptyTextureArray(GLuint x, GLuint y, GLuint z, GLenum internal_format, GLenum format, GLenum type, GLenum min_filter, GLenum mag_filter, bool skip_zero_fill){
	this->internal_format = internal_format;
	this->texture_format = format;
	this->texture_type = type;
	this->min_filter = min_filter;
	this->mag_filter = mag_filter;
	this->x = x;
	this->y = y;
	this->z = z;
	
	GLenum bind_target = (y > 1 && x > 1) ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_1D_ARRAY;
	GLenum get_target = (y > 1 && x > 1) ? GL_TEXTURE_BINDING_2D_ARRAY : GL_TEXTURE_BINDING_1D_ARRAY;

	COURTEOUS_BIND(ID(), get_target, bind_target);
	CHECKED_GL_CALL(glTexStorage3D(bind_target, 1, internal_format, x, y, z));
	if(!skip_zero_fill){
		CHECKED_GL_CALL(glClearTexImage(ID(), 0, format, type, pixel_sample));
	}
	CHECKED_GL_CALL(initParams());
	COURTEOUS_UNBIND(bind_target);
}

void TextureArray::loadImageIntoLayer(const Image& image, GLuint layer, GLint x_offset, GLint y_offset){
	assert(x != 0 && y != 0 && z != 0);
	assert(static_cast<GLint>(layer) < z);
	if(image.getWidth() + x_offset > x || image.getHeight() + y_offset > y){
		fprintf(stderr, "Warning (TextureArray::loadImageIntoLayer): Image does not fit in layer and will be clipped");
	}

	GLsizei realwidth = MAX(0, MIN(image.getWidth(), x-x_offset));
	GLsizei realheight = MAX(0, MIN(image.getHeight(), y-y_offset));

	GLenum bind_target = (y > 1 && x > 1) ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_1D_ARRAY;
	GLenum get_target = (y > 1 && x > 1) ? GL_TEXTURE_BINDING_2D_ARRAY : GL_TEXTURE_BINDING_1D_ARRAY;

	COURTEOUS_BIND(ID(), get_target, bind_target);
	CHECKED_GL_CALL(glTexSubImage3D(bind_target, 0, x_offset, y_offset, layer, realwidth, realheight, 1, texture_format, texture_type, image.data));
	COURTEOUS_UNBIND(bind_target);
}