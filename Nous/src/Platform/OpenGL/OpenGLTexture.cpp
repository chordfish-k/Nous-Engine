#include "pch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"

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
    }

    OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification)
        : m_Specification(specification), m_Width(m_Specification.Width), m_Height(m_Specification.Height)
    {
        NS_PROFILE_FUNCTION();

        m_InternalFormat = Utils::ImageFormatToGLInternalFormat(m_Specification.Format);
        m_DataFormat = Utils::ImageFormatToGLDataFormat(m_Specification.Format);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    Nous::OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
        : m_Path(path)
    {
        NS_PROFILE_FUNCTION();

        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);
        stbi_uc* data = nullptr;

        {
            NS_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std:string&)");
            data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        }
        if (!data)
        {
            NS_CORE_ERROR("{}", path);
            NS_CORE_ASSERT(false, "加载图像失败！")
        }

        m_IsLoaded = true;

        m_Width = width;
        m_Height = height;

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

        m_InternalFormat = channels == 4 ? GL_RGBA8 : (channels == 3 ? GL_RGB8 : 0);
        m_DataFormat = channels == 4 ? GL_RGBA : (channels == 3 ? GL_RGB : 0);

        NS_CORE_ASSERT(m_InternalFormat & m_DataFormat, "图像格式不支持！");

        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    Nous::OpenGLTexture2D::~OpenGLTexture2D()
    {
        NS_PROFILE_FUNCTION();

        glDeleteTextures(1, &m_RendererID);
    }

    void OpenGLTexture2D::SetData(void* data, uint32_t size)
    {
        NS_PROFILE_FUNCTION();

        uint32_t  bpc = m_DataFormat == GL_RGBA ? 4 : 3;
        NS_CORE_ASSERT(size == m_Width * m_Height * bpc, "数据大小必须是整张纹理！");
        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
    }

    void Nous::OpenGLTexture2D::Bind(uint32_t slot) const
    {
        NS_PROFILE_FUNCTION();

        glBindTextureUnit(slot, m_RendererID);
    }
}

