/**
 * \file
 * \author Rudy Castan
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */


#include "PostProcessingPipeline.h"

#include "OpenGL/GL.h"
#include "OpenGL/Buffer.h"
#include "OpenGL/VertexArray.h"
#include <array>
#include <iostream>

PostProcessingPipeline::~PostProcessingPipeline()
{
    Shutdown();
}

void PostProcessingPipeline::Initialize(int width, int height)
{
    currentWidth  = width;
    currentHeight = height;

    setupFullscreenTriangle();

    for (auto& effect : effects)
    {
        createFramebuffer(effect);
    }
}

void PostProcessingPipeline::AddEffect(PostProcessingEffect&& effect)
{
    effects.push_back(std::move(effect));

    if (currentWidth > 0 && currentHeight > 0)
    {
        createFramebuffer(effects.back());
    }
}

OpenGL::TextureHandle PostProcessingPipeline::Apply(OpenGL::TextureHandle input_texture)
{
    OpenGL::TextureHandle current_texture = input_texture;

    for (const auto& effect : effects)
    {
        if (effect.Enabled == PostProcessingEffect::Enable::True && effect.Framebuffer)
        {
            renderEffect(effect, current_texture);
            current_texture = effect.Framebuffer->GetTexture();
        }
    }

    return current_texture;
}

void PostProcessingPipeline::Resize(int width, int height)
{
    currentWidth  = width;
    currentHeight = height;

    for (auto& effect : effects)
    {
        if (effect.Framebuffer)
        {
            effect.Framebuffer->Resize(currentWidth, currentHeight);
        }
    }
}

void PostProcessingPipeline::Shutdown()
{
    for (auto& effect : effects)
    {
        if (effect.Framebuffer)
        {
            effect.Framebuffer->Shutdown();
            effect.Framebuffer.reset();
        }
        if (effect.Shader.Shader != 0)
        {
            glDeleteProgram(effect.Shader.Shader);
            effect.Shader.Shader = 0;
        }
    }
    effects.clear();

    if (fullscreenVAO != 0)
    {
        glDeleteVertexArrays(1, &fullscreenVAO);
        fullscreenVAO = 0;
    }
    if (fullscreenVBO != 0)
    {
        glDeleteBuffers(1, &fullscreenVBO);
        fullscreenVBO = 0;
    }
}

PostProcessingEffect* PostProcessingPipeline::GetEffect(const std::string& name)
{
    for (auto& effect : effects)
    {
        if (effect.Name == name)
        {
            return &effect;
        }
    }
    return nullptr;
}

void PostProcessingPipeline::createFramebuffer(PostProcessingEffect& effect)
{
    if (!effect.Framebuffer)
    {
        effect.Framebuffer = std::make_unique<OffscreenFramebuffer>();
    }

    effect.Framebuffer->Initialize(currentWidth, currentHeight, OffscreenFramebuffer::MSAA::False);
}

void PostProcessingPipeline::renderEffect(const PostProcessingEffect& effect, GLuint input_texture)
{
    effect.Framebuffer->BindForRendering();
    GL::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    GL::Clear(GL_COLOR_BUFFER_BIT);
    GL::Viewport(0, 0, currentWidth, currentHeight);

    GL::UseProgram(effect.Shader.Shader);

    effect.SetUniforms(effect.Shader);

    GL::ActiveTexture(GL_TEXTURE0);
    GL::BindTexture(GL_TEXTURE_2D, input_texture);
    GL::Uniform1i(effect.Shader.UniformLocations.at("uColorTexture"), 0);

    GL::BindVertexArray(fullscreenVAO);
    GL::DrawArrays(GL_TRIANGLES, 0, fullscreenVertexCount);
    GL::BindVertexArray(0);
    GL::BindTexture(GL_TEXTURE_2D, 0);
    GL::UseProgram(0);
    GL::BindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessingPipeline::setupFullscreenTriangle()
{
    struct FullscreenVertex
    {
        float x, y;
        float u, v;
    };

    const FullscreenVertex vertices[] = {
        { -1.0f, -1.0f, 0.0f, 0.0f },
        {  3.0f, -1.0f, 2.0f, 0.0f },
        { -1.0f,  3.0f, 0.0f, 2.0f },
    };

    fullscreenVertexCount = static_cast<GLsizei>(std::ssize(vertices));

    fullscreenVBO = OpenGL::CreateBuffer(OpenGL::BufferType::Vertices, std::as_bytes(std::span{ vertices }));

    const auto layout = {
        OpenGL::VertexBuffer{ 
            fullscreenVBO, 
            { 
                OpenGL::Attribute::Float2, // Position
                OpenGL::Attribute::Float2  // UV
            } 
        }
    };

    fullscreenVAO = OpenGL::CreateVertexArrayObject(layout);
}
