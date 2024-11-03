#include "pch.h"
#include "OpenGLTexture.h"

#include <glad/glad.h>

namespace Nous 
{
    namespace Utils
    {
        static GLenum ImageFormatToGLDataFormat(ImageFormat format)
        {
            switch (format)
            {
            case ImageFormat::RGB8: return GL_RGB;
            case ImageFormat::RGBA8: return GL_RGBA;
            }
            NS_CORE_ASSERT(false);
            return 0;
        }

        static GLenum ImageFormatToGLInternalFormat(ImageFormat format)
        {
            switch (format)
            {
            case ImageFormat::RGB8: return GL_RGB8;
            case ImageFormat::RGBA8: return GL_RGBA8;
            }
            NS_CORE_ASSERT(false);
            return 0;
        }

        static GLint ImageFilterToGLTextureFilter(ImageFilter format)
        {
            switch (format)
            {
            case ImageFilter::Linear: return GL_LINEAR;
            case ImageFilter::Nearst: return GL_NEAREST;
            }
            NS_CORE_ASSERT(false);
            return 0;
        }
    }

    OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification, Buffer data)
        : m_Specification(specification), m_Width(m_Specification.Width), m_Height(m_Specification.Height)
    {
        NS_PROFILE_FUNCTION();

        m_InternalFormat = Utils::ImageFormatToGLInternalFormat(m_Specification.Format);
        m_DataFormat = Utils::ImageFormatToGLDataFormat(m_Specification.Format);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, Utils::ImageFilterToGLTextureFilter(specification.MinFilter));
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, Utils::ImageFilterToGLTextureFilter(specification.MaxFilter));

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);


        if (data)
            SetData(data);
    }

    Nous::OpenGLTexture2D::~OpenGLTexture2D()
    {
        NS_PROFILE_FUNCTION();

        glDeleteTextures(1, &m_RendererID);
    }

    void OpenGLTexture2D::SetData(Buffer data)
    {
        NS_PROFILE_FUNCTION();

        uint32_t  bpc = m_DataFormat == GL_RGBA ? 4 : 3;
        NS_CORE_ASSERT(data.Size == m_Width * m_Height * bpc, "数据大小必须是整张纹理！");
        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data.Data);
    }

    void Nous::OpenGLTexture2D::Bind(uint32_t slot) const
    {
        NS_PROFILE_FUNCTION();

        glBindTextureUnit(slot, m_RendererID);
    }
}

