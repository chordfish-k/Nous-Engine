#pragma once
#include "Nous/Renderer/Framebuffer.h"

namespace Nous {

    class OpenGLFramebuffer : public Framebuffer
    {
    public:
        OpenGLFramebuffer(const FramebufferSpecification& spec);
        virtual ~OpenGLFramebuffer();

        void Invalidate();

        virtual void Bind() override;
        virtual void Unbind() override;

        virtual void Resize(uint32_t width, uint32_t height) override;

        virtual uint32_t GetColorAttachmentRendererID() const override { return m_ColorAttachment; };

        virtual const FramebufferSpecification& GetSpecification() const override;
    private:
        uint32_t m_RendererID = 0;
        uint32_t m_ColorAttachment = 0, m_DepthAttachment = 0; // 缓冲区渲染到的颜色缓冲和深度缓冲
        FramebufferSpecification m_Specification;
    };

}
