#pragma once
#ifndef _KJY_TEXTURE_H_
#define _KJY_TEXTURE_H_

#include <glad/glad.h>
#include <memory>
#include "OglObject.hpp"
#include "Image.hpp"

class TextureKole : public OglObject {
public:
	TextureKole(){
		coreobj = make_shared<OglName>(destroyGLTexture);
		glGenTextures(1, getIdPtr());
	}

	static void destroyGLTexture(OglName& self){glDeleteTextures(1, &self);}

	virtual void copyParametersFrom(const TextureKole& other);

	const GLuint getXdim() const {return(x);}
	const GLuint getYdim() const {return(y);}
	const GLuint getZdim() const {return(z);}

	const GLenum* getTextureExtensionMode() const { return(texture_extension_mode); }
	const GLenum getDepthStencilMode() const { return(depth_stencil_mode); }
	const GLenum getMinFilter() const { return(min_filter); }
	const GLenum getMagFilter() const { return(mag_filter); }
	const GLenum getFormat() const { return(texture_format);}
	const GLenum getInternalFormat() const { return(internal_format);}
	const GLenum getType() const {return(texture_type);}

	void setTextureExtensionMode(GLenum mode) { for (int i = 0; i < 3; i++) { texture_extension_mode[i] = mode; } initParams(); }
	void setTextureExtensionModeTuple(const GLenum* tuple) {
		texture_extension_mode[0] = tuple[0];
		texture_extension_mode[1] = tuple[1];
		texture_extension_mode[2] = tuple[2];
		initParams();
	}
	void setDepthStencilMode(GLenum mode) {depth_stencil_mode = mode; initParams();}
	void setMinFilter(GLenum filter) {min_filter = GL_LINEAR; initParams();}
	void setMagFilter(GLenum filter) {mag_filter = GL_LINEAR; initParams();}
protected:
	virtual void initParams();

	GLint x = 0, y = 0, z = 0;
	GLenum internal_format = 0;
	GLenum texture_format = 0;
	GLenum texture_type = 0;

	GLenum texture_extension_mode[3] = { GL_REPEAT, GL_REPEAT, GL_REPEAT };
	GLenum depth_stencil_mode = GL_DEPTH_COMPONENT;
	GLenum min_filter = GL_LINEAR;
	GLenum mag_filter = GL_LINEAR;
};

class SimpleTexture2D : public TextureKole{
public:
	SimpleTexture2D() : TextureKole(){}
	SimpleTexture2D(const Image& image, GLenum internal_format = GL_RGB8, GLenum format = GL_RGB, GLenum type = GL_UNSIGNED_BYTE);
	SimpleTexture2D(GLuint x, GLuint y, GLenum internal_format = GL_RGB8, GLenum format = GL_RGB, GLenum type = GL_UNSIGNED_BYTE) : TextureKole(){
		makeEmpty2DTexture(x, y, internal_format, format, type);
	}

	void makeTextureFromImage(const Image& image, GLenum internal_format = GL_RGB8, GLenum format = GL_RGB, GLenum type = GL_UNSIGNED_BYTE);
	void copyDataFromImage(const Image& image, int x_offset, int y_offset);
	void makeEmpty2DTexture(
		GLuint x, GLuint y, GLenum internal_format = GL_RGB8,
		GLenum format = GL_RGB, GLenum type = GL_UNSIGNED_BYTE, GLenum min_filter = GL_LINEAR,
		GLenum mag_filter = GL_LINEAR, bool skip_zero_fill = false
	);

	void fillSolid(const GLubyte* sample);

	Image* extractImage() const;
	Image* extractImage(int channels) const;
};

class TextureArray : public TextureKole{
public:
	TextureArray() : TextureKole(){}
	TextureArray(GLuint x, GLuint y, GLuint z, GLenum internal_format = GL_RGB8, GLenum format = GL_RGB, GLenum type = GL_UNSIGNED_BYTE){
		makeEmptyTextureArray(x, y, z, internal_format, format, type);
	}

	void makeEmptyTextureArray(GLuint x, GLuint y, GLuint z, GLenum internal_format = GL_RGB8,
		GLenum format = GL_RGB, GLenum type = GL_UNSIGNED_BYTE, GLenum min_filter = GL_LINEAR,
		GLenum mag_filter = GL_LINEAR, bool skip_zero_fill = false
	);

	void loadImageIntoLayer(const Image& image, GLuint layer, GLint x_offset = 0, GLint y_offset = 0);
};

// class TextureFormatMismatch : public std::exception{
//  public:
// 	TextureFormatMismatch(const char* hint){
// 		size_t baselen = strlen(basestr);
// 		this->hint = new char[MIN(strlen(hint), 100+baselen)];
// 		strcpy(this->hint, basestr);
// 		strncat(this->hint, hint, 100+baselen);
// 	}
// 	~TextureFormatMismatch(){delete [] this->hint;}
// 	virtual const char* what() const throw(){
// 		return(hint);
// 	}
//  private:
// 	const char* basestr = "TextureKole Formats were not compatible: %s";
// 	char* hint = nullptr;
// };

#endif