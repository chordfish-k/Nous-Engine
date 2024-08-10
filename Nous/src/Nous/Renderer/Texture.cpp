#include "pch.h"
#include "Texture.h"

#include "Nous/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Nous {


    Ref <Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: NS_CORE_ASSERT(false, "RendererAPI 未选择");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return CreateRef<OpenGLTexture2D>(width, height);
        }

        NS_CORE_ASSERT(false, "RendererAPI 未知")
        return nullptr;
    }

    Ref <Texture2D> Texture2D::Create(const std::string& path)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: NS_CORE_ASSERT(false, "RendererAPI 未选择");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return CreateRef<OpenGLTexture2D>(path);
        }

        NS_CORE_ASSERT(false, "RendererAPI 未知")
        return nullptr;
    }
}