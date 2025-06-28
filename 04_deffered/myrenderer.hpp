#pragma once

#include <vector>

#include "camera.hpp"
#include "error_handling.hpp"
#include "spotlight.hpp"
#include "framebuffer.hpp"
#include "shadowmap_framebuffer.hpp"
#include "ogl_material_factory.hpp"
#include "ogl_geometry_factory.hpp"
#include "ssao.hpp"

struct ColorAttachment {
    GLuint id;
    GLenum tag;
};

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

    void CreateColorAttachment(ColorAttachment& attch, GLint internalFormat, GLint format, GLint type, int width, int height) {
        GL_CHECK(glGenTextures(1, &attch.id));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, attch.id));

        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL));

        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

        attch.tag = GL_COLOR_ATTACHMENT0 + _attachementsCreated;

        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, attch.tag, GL_TEXTURE_2D, attch.id, 0);)
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
    }
};