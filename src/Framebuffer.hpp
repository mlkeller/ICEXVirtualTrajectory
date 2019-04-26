#pragma once
#ifndef _KJY_FRAMEBUFFEROBJECT_H_
#define _KJY_FRAMEBUFFEROBJECT_H_

#include <glad/glad.h>
#include <vector>
#include <stack>
#include "OglObject.hpp"
#include "TextureKole.hpp"

class Framebuffer : public OglObject {
public:
	Framebuffer(){
        coreobj = make_shared<OglName>(destroyFBO);
        glGenFramebuffers(1, getIdPtr());
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);
        numbered_attachment_references.resize(maxAttach);
    }

    static void destroyFBO(OglName& self){
		glDeleteFramebuffers(1, &self);
    }

    void attachRenderbuffer(GLuint renderbuffer, GLenum attachment, bool clobber = false);
    void attach(const SimpleTexture2D& tex2d, GLenum attachment, int level = 0, bool clobber = false);
    void attach(const TextureArray& texarray, GLenum attachment, int layer, int level = 0);
    /* TODO: Make Renderbuffer class */ // void attatch(const Renberbuffer&)

    void setDrawBuffers(const std::vector<GLenum>& bufs, bool clobber = false);
    void setDrawBuffers(GLsizei n, const GLenum *bufs, bool clobber = false);

    void setSingleDrawMode(GLenum mode);
    void setSingleReadMode(GLenum mode);

    void setDefaultWidth(GLint width);
    GLint getDefaultWidth() const {return(default_width);}
    void setDefaultHeight(GLint height);
    GLint getDefaultHeight() const {return(default_height);}
    void setDefaultLayers(GLint layers);
    GLint getDefaultLayers() const {return(default_layers);}
    void setDefaultSamples(GLint samples);
    GLint getDefaultSamples() const {return(default_samples);}
    void setDefaultFixedSampleLocations(GLint param);
    GLint getDefaultFixedSampleLocations() const {return(default_fixed_sample_locations);}

    void bind() const {
        CHECKED_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, ID()));
    }
    void bindDraw() const {
        CHECKED_GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ID()));
    }
    void bindRead() const {
        CHECKED_GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, ID()));
    }

    GLenum getStatus(GLenum target = GL_FRAMEBUFFER) const {
        return(glCheckNamedFramebufferStatus(ID(), GL_FRAMEBUFFER));
    }

    bool isComplete() const {
        return(getStatus() == GL_FRAMEBUFFER_COMPLETE);
    }

    static size_t get_stashed_id_count(){return(stashed_read_draw_ids.size());}

    static void push_framebuffer_state();

    static void pop_framebuffer_state();

protected:
    void storeReference(GLenum attachment, const OglObject& globj);
    /* OglObject instances to maintain reference counting on attached objects*/
    GLint maxAttach = 0;
    std::vector<PersistentOglName> numbered_attachment_references;
    PersistentOglName depth_attachment_ref;
    PersistentOglName depth_stencil_attachment_ref;
    PersistentOglName stencil_attachment_ref;

    // I don't actually know the defaults for these so -1 for now
    GLint default_width = -1;
    GLint default_height = -1;
    GLint default_layers = -1;
    GLint default_samples = -1;
    GLint default_fixed_sample_locations = -1;

    static std::stack<std::pair<GLint, GLint>> stashed_read_draw_ids;
};

#endif