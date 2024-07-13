#include "pch.h"
#include "VertexArray.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Nous {

    VertexArray* VertexArray::Create()
    {
        switch (Renderer::GetAPI()) {
            case RendererAPI::None:
                NS_CORE_ASSERT(false, "RendererAPI::None 不支持");
                return nullptr;
            case RendererAPI::OpenGL:
                return new OpenGLVertexArray();
        }
        NS_CORE_ASSERT(false, "RendererAPI 未知");
        return nullptr;
    }

}