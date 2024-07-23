#include "pch.h"
#include "VertexArray.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Nous {

    Ref<VertexArray> VertexArray::Create()
    {
        switch (Renderer::GetAPI()) {
            case RendererAPI::API::None:
                NS_CORE_ASSERT(false, "RendererAPI::None 不支持");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return std::make_shared<OpenGLVertexArray>();
        }
        NS_CORE_ASSERT(false, "RendererAPI 未知");
        return nullptr;
    }

}