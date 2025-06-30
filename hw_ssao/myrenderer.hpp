#pragma once

#include <glad/glad.h>
#include <memory>
#include <vector>

#include "error_handling.hpp"
#include "material_factory.hpp"
#include "ogl_geometry_construction.hpp"
#include "ogl_geometry_factory.hpp"
#include "ogl_material_factory.hpp"
#include "scene_object.hpp"
#include "shader.h"
#include "ssao.hpp"

class MySceneObject {
public:
    MySceneObject(const std::string& meshPath, std::shared_ptr<OGLTexture> texture) :
        _texture(texture)
    {
        OGLGeometryFactory geometry_factory;
        _geometry = static_pointer_cast<OGLGeometry>(
            geometry_factory.loadMesh(meshPath, RenderStyle::Solid)
        );
    }

    const OGLGeometry& geometry() const {
        return *_geometry;
    }

    const OGLTexture& texture() const {
        return *_texture;
    }

private:
    std::shared_ptr<OGLGeometry> _geometry;
    std::shared_ptr<OGLTexture> _texture;
};

class MyScene {
public:
    OGLMaterialFactory* _materialFactory;

    MyScene(OGLMaterialFactory* materialFactory) : 
        _materialFactory(materialFactory) 
    {}

    MyScene& addObject(const std::string& meshPath, const std::string& texturePath) {
        _objects.emplace_back(
            meshPath,
            static_pointer_cast<OGLTexture>(_materialFactory->getTexture(texturePath))
        );
        return *this;
    }

    const std::vector<MySceneObject>& getObjects() const {
        return _objects;
    }

private:
    std::vector<MySceneObject> _objects;
};


class MyRenderer {
public:
    MyRenderer(int screenWidth, int screenHeight) :
        _geometryShader("./shaders/geometry.vs.glsl", "./shaders/geometry.fs.glsl"),
        _ssaoShader("./shaders/pass.vs.glsl", "./shaders/ssao.fs.glsl"),
        _blurShader("./shaders/pass.vs.glsl", "./shaders/blur.fs.glsl"),
        _lightingShader("./shaders/pass.vs.glsl", "./shaders/lighting.fs.glsl"),
        _quad(generateQuadTex())
    {
        Resize(screenWidth, screenHeight);
    }

    void Resize(int width, int height) {
        _screenWidth = width;
        _screenHeight = height;

        CreateFramebuffer(_gBuffer);
        CreateColorAttachmentTex(_gAlbedo, GL_RGBA, GL_RGBA, GL_FLOAT);
        CreateColorAttachmentTex(_gNormal, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        CreateColorAttachmentTex(_gPosition, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        AttachTextures(_gBuffer, {_gAlbedo, _gNormal, _gPosition});

        CreateFramebuffer(_ssaoBuffer);
        CreateColorAttachmentTex(_ssaoTex, GL_RED, GL_RED, GL_FLOAT);
        AttachTextures(_ssaoBuffer, {_ssaoTex});

        CreateFramebuffer(_blurBuffer);
        CreateColorAttachmentTex(_blurredSsaoTex, GL_RED, GL_RED, GL_FLOAT);
        AttachTextures(_blurBuffer, {_blurredSsaoTex});

        CreateFramebuffer(_debugBuffer);
        CreateColorAttachmentTex(_debugTex1, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        CreateColorAttachmentTex(_debugTex2, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        CreateColorAttachmentTex(_debugTex3, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        CreateColorAttachmentTex(_debugTex4, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        AttachTextures(_debugBuffer, {_debugTex1, _debugTex2, _debugTex3, _debugTex4});

        CreateDepthBuffer(_gBuffer, depthBuffer);
    }

    template<typename TCamera>
    void GeometryPass(const MyScene &scene, const TCamera &camera) {
        BindFramebuffer(_gBuffer);
        Clear();
        GL_CHECK(glEnable(GL_DEPTH_TEST));
		GL_CHECK(glViewport(0, 0, _screenWidth, _screenHeight));

        _geometryShader.use();

        _geometryShader.setMat4("u_viewMat", camera.getViewMatrix());
        _geometryShader.setMat4("u_projMat", camera.getProjectionMatrix());

        for (auto&& object : scene.getObjects()) {
            const OGLGeometry& geometry = object.geometry();
            const OGLTexture& texture = object.texture();

            BindShaderTexture(_geometryShader, "u_diffuseTexture", texture.texture.get());

            geometry.bind();
            geometry.draw();
        }

        UnbindFramebuffer();
    }

    template<typename TCamera>
    void SsaoPass(const TCamera& camera) {
        BindFramebuffer(_ssaoBuffer);
        Clear();
        GL_CHECK(glDisable(GL_DEPTH_TEST));

        _ssaoShader.use();

        BindShaderTexture(_ssaoShader, "u_normal", _gNormal);
        BindShaderTexture(_ssaoShader, "u_position", _gPosition);

        // Pass SSAO samples
        GLint location = glGetUniformLocation(_ssaoShader.ID, "u_ssaoSamples");
		GL_CHECK(glUniform3fv(location, ssao::SAMPLES_COUNT, glm::value_ptr(_ssao.kernel()[0])));

        _ssaoShader.setBool("u_enableSSAO", _renderSSAO);
        _ssaoShader.setMat4("u_projMat", camera.getProjectionMatrix());

        RenderQuad();
        UnbindFramebuffer();
    }

    void BlurPass() {
        BindFramebuffer(_blurBuffer);
        Clear();
        GL_CHECK(glDisable(GL_DEPTH_TEST));

        _blurShader.use();

        BindShaderTexture(_blurShader, "u_ssaoBuffer", _ssaoTex);
        _blurShader.setBool("u_enableSSAO", _renderSSAO);

        RenderQuad();
        UnbindFramebuffer();
    }

    template<typename TLight>
    void LightingPass(const TLight& light) {
        // BindFramebuffer(_debugBuffer);
        Clear();
        GL_CHECK(glDisable(GL_DEPTH_TEST));

        _lightingShader.use();

        BindShaderTexture(_lightingShader, "u_albedo", _gAlbedo);
        BindShaderTexture(_lightingShader, "u_normal", _gNormal);
        BindShaderTexture(_lightingShader, "u_position", _gPosition);
        BindShaderTexture(_lightingShader, "u_ssao", _blurredSsaoTex);

        _lightingShader.setBool("u_enableAlbedo", _renderAlbedo);
        _lightingShader.setVec3("u_lightPos", light.getPosition());
        _lightingShader.setMat4("u_lightViewMat", light.getViewMatrix());
        _lightingShader.setMat4("u_lightProjMat", light.getProjectionMatrix());

        RenderQuad();
        UnbindFramebuffer();
    }

    void ToggleAlbedo() {
        _renderAlbedo = !_renderAlbedo;
    }

    void ToggleSSAO() {
        _renderSSAO = !_renderSSAO;
    }

private:
    int _screenWidth;
    int _screenHeight;

    int _attachementsCreated = 0;
    IndexedBuffer _quad;

    GLuint _gBuffer, _ssaoBuffer, _blurBuffer, _debugBuffer, depthBuffer;
    GLuint _gPosition, _gNormal, _gAlbedo;
    GLuint _ssaoTex, _blurredSsaoTex;
    GLuint _debugTex1, _debugTex2, _debugTex3, _debugTex4;
    Shader _geometryShader, _ssaoShader, _blurShader, _lightingShader;

    ssao _ssao;

    bool _renderAlbedo = true;
    bool _renderSSAO = true;

    void RenderQuad() {
        GL_CHECK(glBindVertexArray(_quad.vao.get()));
  		GL_CHECK(glDrawElements(_quad.mode, _quad.indexCount, GL_UNSIGNED_INT, reinterpret_cast<void*>(0)));
    }

    void CreateFramebuffer(GLuint& buffer) {
        GL_CHECK(glGenFramebuffers(1, &buffer));
    }

    void BindFramebuffer(GLuint buffer) {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, buffer));
    }

    void UnbindFramebuffer() {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    void BindShaderTexture(const Shader& shader, const std::string uniformName, GLuint texture) {
        GLuint location = GL_CHECK(glGetUniformLocation(shader.ID, uniformName.c_str()));
        shader.setInt(uniformName, location);
        GL_CHECK(glActiveTexture(GL_TEXTURE0 + location));
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
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer));
        std::vector<GLenum> drawBuffers;

        for ( int i = 0; i < attachements.size(); ++i) {
            auto attch = attachements[i];
            GLint drawBuffer = GL_COLOR_ATTACHMENT0 + i;
            drawBuffers.push_back(drawBuffer);

            GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, drawBuffer, GL_TEXTURE_2D, attch, 0));
        }

        GL_CHECK(glDrawBuffers(drawBuffers.size(), drawBuffers.data()));

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            throw OpenGLError("Framebuffer not complete: " + std::to_string(status));
        }

        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER,0));
    }

    void CreateDepthBuffer(GLuint framebuffer, GLuint& depthBuffer) {
        GL_CHECK(glGenRenderbuffers(1, &depthBuffer));
        GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer));
        GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _screenWidth, _screenHeight));

        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer));
        GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer));
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    void Clear() {
		GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }
};