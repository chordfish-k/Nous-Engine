#include "pch.h"
#include "Buffer.h"

#include "Renderer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Nous {


    VertexBuffer *VertexBuffer::Create(float *vertices, uint32_t size)
    {
        switch (Renderer::GetAPI()) {
            case RendererAPI::None:
                NS_CORE_ASSERT(false, "RendererAPI 未选择");
                return nullptr;
            case RendererAPI::OpenGL:
                return new OpenGLVertexBuffer(vertices, size);
        }

        NS_CORE_ASSERT(false, "未知的RendererAPI")
        return nullptr;
    }

    IndexBuffer *IndexBuffer::Create(uint32_t *indices, uint32_t size)
    {
        switch (Renderer::GetAPI()) {
            case RendererAPI::None:
                NS_CORE_ASSERT(false, "RendererAPI 未选择");
                return nullptr;
            case RendererAPI::OpenGL:
                return new OpenGLIndexBuffer(indices, size);
        }

        NS_CORE_ASSERT(false, "未知的RendererAPI")
        return nullptr;
    }
}
