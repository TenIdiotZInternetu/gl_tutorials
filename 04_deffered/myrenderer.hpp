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
    MyRenderer(int screenWidth, int screenHeight) :
        _screenWidth(screenWidth),
        _screenHeight(screenHeight)
    {}

    void init() {
        _quad = generateQuadTex();

        CreateFramebuffer(_gBuffer);
        GLuint gPosition, gNormal, gAlbedo;

        CreateColorAttachmentTex(gPosition, GL_RGBA, GL_RGBA, GL_FLOAT);
        CreateColorAttachmentTex(gNormal, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        CreateColorAttachmentTex(gAlbedo, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        AttachTextures(_gBuffer, {gPosition, gNormal, gAlbedo});

        CreateFramebuffer(_debugBuffer);
        GLuint fb0, fb1, fb2, fb3;

        CreateColorAttachmentTex(fb0, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        CreateColorAttachmentTex(fb1, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        CreateColorAttachmentTex(fb2, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        CreateColorAttachmentTex(fb3, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        AttachTextures(_debugBuffer, {fb0, fb1, fb2, fb3});

        CreateDepthBuffer(_gBuffer, depthBuffer);
    }

private:
    int _screenWidth;
    int _screenHeight;

    int _attachementsCreated = 0;
    IndexedBuffer _quad;

    GLuint _gBuffer, _debugBuffer, depthBuffer;

    void CreateFramebuffer(GLuint& buffer) {
        GL_CHECK(glGenBuffers(1, &buffer));
    }

    void CreateColorAttachmentTex(GLuint& attch, GLint internalFormat, GLint format, GLint type) {
        CreateColorAttachmentTex(attch, internalFormat, format, type, _screenWidth, _screenHeight);
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

    void CreateDepthBuffer(GLuint framebuffer, GLuint& depthBuffer) {
        GL_CHECK(glGenRenderbuffers(1, &depthBuffer));
        GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer));
        GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _screenWidth, _screenHeight));

        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer));
        GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer));
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
};