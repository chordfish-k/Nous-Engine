#include "pch.h"
#include "UniformBuffer.h"

#include "Nous/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"

namespace Nous {

    Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:    NS_CORE_ASSERT(false, "RendererAPI 未选择"); return nullptr;
            case RendererAPI::API::OpenGL:  return CreateRef<OpenGLUniformBuffer>(size, binding);
        }

        NS_CORE_ASSERT(false, "未知的 RendererAPI!");
        return nullptr;
    }

}