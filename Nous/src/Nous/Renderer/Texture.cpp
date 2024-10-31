#include "pch.h"
#include "Texture.h"

#include "Nous/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Nous {


    Ref <Texture2D> Texture2D::Create(const TextureSpecification& specification, Buffer data)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: NS_CORE_ASSERT(false, "RendererAPI 未选择"); return nullptr;
            case RendererAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(specification, data);
        }
        NS_CORE_ASSERT(false, "RendererAPI 未知")
        return nullptr;
    }
}