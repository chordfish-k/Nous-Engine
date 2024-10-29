#include "pch.h"
#include "Texture.h"

#include "Nous/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Nous {


    Ref <Texture2D> Texture2D::Create(const TextureSpecification& specification)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: NS_CORE_ASSERT(false, "RendererAPI 未选择"); return nullptr;
            case RendererAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(specification);
        }
        NS_CORE_ASSERT(false, "RendererAPI 未知")
        return nullptr;
    }

    Ref <Texture2D> Texture2D::Create(const std::string& path)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: NS_CORE_ASSERT(false, "RendererAPI 未选择"); return nullptr;
            case RendererAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(path);
        }
        NS_CORE_ASSERT(false, "RendererAPI 未知")
        return nullptr;
    }
}