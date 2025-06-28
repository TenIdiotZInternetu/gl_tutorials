#pragma once

#include <glad/glad.h>
#include <vector>

#include "camera.hpp"
#include "error_handling.hpp"
#include "spotlight.hpp"
#include "ogl_material_factory.hpp"
#include "ogl_geometry_factory.hpp"
#include "ssao.hpp"

class MyRenderer {
public:
private:
    IndexedBuffer _quad;
    int _screenWidth;
    int _screenHeight;
    int _attachementsCreated = 0;

    void CreateFrameBuffer(GLuint& buffer) {
        GL_CHECK(glGenBuffers(1, &buffer));
    }

    void CreateColorAttachment(GLuint& attch, GLint internalFormat, GLint type, GLint format) {
        CreateColorAttachment(attch, internalFormat, type, format);
    }

    void CreateColorAttachmentTex(GLuint& attch, GLint internalFormat, GLint format, GLint type, int width, int height) {
        GL_CHECK(glGenTextures(1, &attch));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, attch));

        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL));

        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

        GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
    }

    void AttachTextures(GLuint& framebuffer, const std::vector<GLuint>& attachements) {
        GL_CHECK(glBindBuffer(GL_FRAMEBUFFER, framebuffer));

        for ( int i = 0; i < attachements.size(); ++i) {
            auto attch = attachements[i];
            GLint tag = GL_COLOR_ATTACHMENT0 + i;
            GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, tag, GL_TEXTURE_2D, attch, 0));
        }

        GL_CHECK(glBindBuffer(GL_FRAMEBUFFER,0));
    }

    
};