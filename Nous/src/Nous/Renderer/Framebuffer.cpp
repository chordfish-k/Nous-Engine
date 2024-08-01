#include "pch.h"
#include "Framebuffer.h"

#include "Nous/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace Nous {

    Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: NS_CORE_ASSERT(false, "RendererAPI 未选择");
                return nullptr;
            case RendererAPI::API::OpenGL:return CreateRef<OpenGLFramebuffer>(spec);
        }

        NS_CORE_ASSERT(false, "RendererAPI 未知")
        return nullptr;
    }
}
