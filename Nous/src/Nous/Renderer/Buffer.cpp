#include "pch.h"
#include "Buffer.h"

#include "Renderer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Nous {


    Ref <VertexBuffer> VertexBuffer::Create(uint32_t size)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: NS_CORE_ASSERT(false, "RendererAPI 未选择");
                return nullptr;
            case RendererAPI::API::OpenGL:return CreateRef<OpenGLVertexBuffer>(size);
        }

        NS_CORE_ASSERT(false, "RendererAPI 未知")
        return nullptr;
    }

    Ref <VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: NS_CORE_ASSERT(false, "RendererAPI 未选择");
                return nullptr;
            case RendererAPI::API::OpenGL:return std::make_shared<OpenGLVertexBuffer>(vertices, size);
        }

        NS_CORE_ASSERT(false, "RendererAPI 未知")
        return nullptr;
    }

    Ref <IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t size)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: NS_CORE_ASSERT(false, "RendererAPI 未选择");
                return nullptr;
            case RendererAPI::API::OpenGL:return std::make_shared<OpenGLIndexBuffer>(indices, size);
        }

        NS_CORE_ASSERT(false, "RendererAPI 未知")
        return nullptr;
    }
}
