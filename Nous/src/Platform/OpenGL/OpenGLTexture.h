#pragma once

#include "Nous/Renderer/Texture.h"

using GLenum = uint32_t;

namespace Nous {

    class OpenGLTexture2D : public Texture2D
    {
    public:
        OpenGLTexture2D(const TextureSpecification& specification, Buffer data = Buffer());
        virtual ~OpenGLTexture2D() override;

        virtual const TextureSpecification& GetSpecification() const override { return m_Specification; }

        virtual uint32_t GetWidth() const override { return m_Width; };
        virtual uint32_t GetHeight() const override { return m_Height; };
        virtual uint32_t GetRendererID() const override { return m_RendererID; }

        virtual void SetData(Buffer data) override;

        virtual void Bind(uint32_t slot) const override;

        virtual bool IsLoaded() const override { return m_IsLoaded; }

        virtual bool operator==(const Texture& other) const override { return m_RendererID == other.GetRendererID(); };
    private:
        TextureSpecification m_Specification;

        bool m_IsLoaded = false;
        uint32_t m_Width, m_Height;
        uint32_t m_RendererID;
        GLenum m_InternalFormat, m_DataFormat;
    };
}
