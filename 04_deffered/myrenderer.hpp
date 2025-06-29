#pragma once

#include <glad/glad.h>
#include <vector>

#include "shader.h"
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

        _geometryShader = Shader("geometry.vs.glsl", "geometry.fs.glsl");
        _ligthingShader = Shader("lighting.vs.glsl", "lighting.fs.glsl");

        CreateFramebuffer(_gBuffer);
        CreateColorAttachmentTex(_gPosition, GL_RGBA, GL_RGBA, GL_FLOAT);
        CreateColorAttachmentTex(_gNormal, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        CreateColorAttachmentTex(_gAlbedo, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        AttachTextures(_gBuffer, {_gPosition, _gNormal, _gAlbedo});

        CreateFramebuffer(_debugBuffer);
        CreateColorAttachmentTex(_debugTex1, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        CreateColorAttachmentTex(_debugTex2, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        CreateColorAttachmentTex(_debugTex3, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        CreateColorAttachmentTex(_debugTex4, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        AttachTextures(_debugBuffer, {_debugTex1, _debugTex2, _debugTex3, _debugTex4});

        CreateDepthBuffer(_gBuffer, depthBuffer);
    }

    void geometryPass() {
        clear();
        GL_CHECK(glEnable(GL_DEPTH_TEST));
		GL_CHECK(glViewport(0, 0, _screenWidth, _screenHeight));

        BindFramebuffer(_gBuffer);


        UnbindFramebuffer();
    }

private:
    int _screenWidth;
    int _screenHeight;

    int _attachementsCreated = 0;
    IndexedBuffer _quad;

    GLuint _gBuffer, _debugBuffer, depthBuffer;
    GLuint _gPosition, _gNormal, _gAlbedo;
    GLuint _debugTex1, _debugTex2, _debugTex3, _debugTex4;
    Shader _geometryShader, _ligthingShader;

    ssao _ssao;


    void CreateFramebuffer(GLuint& buffer) {
        GL_CHECK(glGenBuffers(1, &buffer));
    }

    void BindFramebuffer(GLuint buffer) {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, buffer));
    }

    void UnbindFramebuffer() {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    void BindShaderTexture(GLuint index, GLuint texture) {
        GL_CHECK(glActiveTexture(GL_TEXTURE0 + index));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));
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

    void AttachTextures(GLuint framebuffer, const std::vector<GLuint>& attachements) {
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

    void clear() {
		UnbindFramebuffer();
		GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }
};