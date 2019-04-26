#include "Framebuffer.hpp"

void Framebuffer::attachRenderbuffer(GLuint renderbuffer, GLenum attachment, bool clobber){
    if(!clobber){
        push_framebuffer_state();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, ID());

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbuffer);


    if(!clobber){
        pop_framebuffer_state();
    }
}

void Framebuffer::attach(const SimpleTexture2D& tex2d, GLenum attachment, int level, bool clobber){
    if(!clobber){
        push_framebuffer_state();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, ID());

    storeReference(attachment, tex2d);

    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, tex2d.ID(), level);

    if(!clobber){
        pop_framebuffer_state();
    }
}

void Framebuffer::attach(const TextureArray& texarray, GLenum attachment, int layer, int level){
    storeReference(attachment, texarray);
    glNamedFramebufferTextureLayer(ID(), attachment, texarray.ID(), level, layer);
}

void Framebuffer::setDrawBuffers(const std::vector<GLenum>& bufs, bool clobber){
    setDrawBuffers(bufs.size(), bufs.data(), clobber);
}

void Framebuffer::setDrawBuffers(GLsizei n, const GLenum *bufs, bool clobber){
    if(!clobber){
        push_framebuffer_state();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, ID());
    glDrawBuffers(n, bufs);

    if(!clobber){
        pop_framebuffer_state();
    }
}

void Framebuffer::setSingleDrawMode(GLenum mode){
    glDrawBuffer(mode);
}

void Framebuffer::setSingleReadMode(GLenum mode){
    glReadBuffer(mode);
}

void Framebuffer::setDefaultWidth(GLint width){
    default_width = width;
    CHECKED_GL_CALL ( glNamedFramebufferParameteri(ID(), GL_FRAMEBUFFER_DEFAULT_WIDTH, default_width) ); 
}
void Framebuffer::setDefaultHeight(GLint height){
    default_height = height;
    CHECKED_GL_CALL ( glNamedFramebufferParameteri(ID(), GL_FRAMEBUFFER_DEFAULT_HEIGHT, default_height) ); 
}
void Framebuffer::setDefaultLayers(GLint layers){
    default_layers = layers;
    CHECKED_GL_CALL ( glNamedFramebufferParameteri(ID(), GL_FRAMEBUFFER_DEFAULT_LAYERS, default_layers) ); 
}
void Framebuffer::setDefaultSamples(GLint samples){
    default_samples = samples;
    CHECKED_GL_CALL ( glNamedFramebufferParameteri(ID(), GL_FRAMEBUFFER_DEFAULT_SAMPLES, default_samples) ); 
}
void Framebuffer::setDefaultFixedSampleLocations(GLint param){
    default_fixed_sample_locations = param;
    CHECKED_GL_CALL ( glNamedFramebufferParameteri(ID(), GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS, default_fixed_sample_locations) ); 
}

std::stack<std::pair<GLint, GLint>> Framebuffer::stashed_read_draw_ids = std::stack<std::pair<GLint, GLint>>();

void Framebuffer::push_framebuffer_state(){
    GLint read_id = 0;
    GLint draw_id = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &read_id);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_id);
    stashed_read_draw_ids.push(std::pair<GLint, GLint>(read_id, draw_id));
}

void Framebuffer::pop_framebuffer_state(){
    std::pair<GLint, GLint> id_pair = stashed_read_draw_ids.top();
    GLint read_id = id_pair.first;
    GLint draw_id = id_pair.second;
    stashed_read_draw_ids.pop();

    if(read_id == draw_id){
        glBindFramebuffer(GL_FRAMEBUFFER, read_id);
    }else{
        glBindFramebuffer(GL_READ_FRAMEBUFFER, read_id);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_id);
    }
}

void Framebuffer::storeReference(GLenum attachment, const OglObject& globj){
    switch(attachment){
        case GL_STENCIL_ATTACHMENT:
            stencil_attachment_ref = globj.coreobj;
            break;
        case GL_DEPTH_ATTACHMENT:
            depth_attachment_ref = globj.coreobj;
            break;
        case GL_DEPTH_STENCIL_ATTACHMENT:
            depth_stencil_attachment_ref = globj.coreobj;
            break;
        default:
            if(attachment < GL_COLOR_ATTACHMENT0 || attachment >= GL_COLOR_ATTACHMENT0 + (GLuint)maxAttach){
                fprintf(stderr, "Warning! FBO attatchment enum for reference storage appears to be invalid\n");
            }else{
                int color_index = attachment - GL_COLOR_ATTACHMENT0;
                numbered_attachment_references[color_index] = globj.coreobj;
            }
            break;
    }
}