#pragma once

#include "Nous/Core/Base.h"

namespace Nous {

    enum class FramebufferTextureFormat
    {
          None = 0,

          // Color
          RGBA8,
          RED_INTEGER, // 只有一个红色通道的整数，相对于int

          // Depth/stencil 模板深度 深度测试+模板缓冲
          DEPTH25STENCIL8,

          // Default
          Depth = DEPTH25STENCIL8
    };

    struct FramebufferTextureSpecification
    {
        FramebufferTextureSpecification() = default;
        FramebufferTextureSpecification(FramebufferTextureFormat format)
            : TextureFormat(format) {}

        FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
    };

    struct FramebufferAttachmentSpecification
    {
        FramebufferAttachmentSpecification() = default;
        FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
            : Attachments(attachments) {}

        std::vector<FramebufferTextureSpecification> Attachments;
    };

    // 帧缓冲规范
    struct FramebufferSpecification
    {
        uint32_t Width, Height;
        FramebufferAttachmentSpecification Attachments;
        uint32_t Samples = 1; // 采样

        bool SwapChainTarget = false;
    };

    class Framebuffer
    {
    public:
        virtual ~Framebuffer() = default;

        virtual void Bind() = 0;
        virtual void Unbind() = 0;

        virtual void Resize(uint32_t width, uint32_t height) = 0;
        virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

        virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;

        virtual uint32_t GetColorAttachmentRendererID(uint32_t index) const = 0;

        virtual const FramebufferSpecification& GetSpecification() const = 0;

        static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
    };

}
