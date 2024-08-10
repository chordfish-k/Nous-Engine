#include "pch.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>

namespace Nous {
    //////////////////
    // VertexBuffer //
    //////////////////
    OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
    {
        NS_PROFILE_FUNCTION();

        glCreateBuffers(1, &m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW); // GL_DYNAMIC_DRAW:顶点缓冲区是动态的
    }


    OpenGLVertexBuffer::OpenGLVertexBuffer(float *vertices, uint32_t size)
    {
        NS_PROFILE_FUNCTION();

        glCreateBuffers(1, &m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer()
    {
        NS_PROFILE_FUNCTION();

        glDeleteBuffers(1, &m_RendererID);
    }

    void OpenGLVertexBuffer::Bind() const
    {
        NS_PROFILE_FUNCTION();

        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    }

    void OpenGLVertexBuffer::Unbind() const
    {
        NS_PROFILE_FUNCTION();

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void OpenGLVertexBuffer::SetData(const void* data, uint32_t size)
    {
        NS_PROFILE_FUNCTION();

        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
    }

    /////////////////
    // IndexBuffer //
    /////////////////
    OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
        : m_Count(count)
    {
        NS_PROFILE_FUNCTION();

        glCreateBuffers(1, &m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer()
    {
        NS_PROFILE_FUNCTION();

        glDeleteBuffers(1, &m_RendererID);
    }

    void OpenGLIndexBuffer::Bind() const
    {
        NS_PROFILE_FUNCTION();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    }

    void OpenGLIndexBuffer::Unbind() const
    {
        NS_PROFILE_FUNCTION();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}
