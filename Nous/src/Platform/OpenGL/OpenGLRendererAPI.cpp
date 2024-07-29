#include "pch.h"
#include "OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace Nous {

    void OpenGLMessageCallback(
        unsigned source,
        unsigned type,
        unsigned id,
        unsigned severity,
        int length,
        const char* message,
        const void* userParam)
    {
        switch (severity)
        {
            case GL_DEBUG_SEVERITY_HIGH:         NS_CORE_CRITICAL(message); return;
            case GL_DEBUG_SEVERITY_MEDIUM:       NS_CORE_ERROR(message); return;
            case GL_DEBUG_SEVERITY_LOW:          NS_CORE_WARN(message); return;
            case GL_DEBUG_SEVERITY_NOTIFICATION: NS_CORE_TRACE(message); return;
        }

        NS_CORE_ASSERT(false, "未知的信息等级!");
    }

    void OpenGLRendererAPI::Init()
    {
        NS_PROFILE_FUNCTION();

#ifdef NS_DEBUG
        glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // c1 * a1 + c2 * (1-a1)

        glEnable(GL_DEPTH_TEST);
    }

    void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        glViewport(x, y, width, height);
    }

    void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void OpenGLRendererAPI::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLRendererAPI::DrawIndexed(const Ref <VertexArray>& vertexArray, uint32_t indexCount)
    {
        uint32_t count = indexCount ? vertexArray->GetIndexBuffer()->GetCount() : indexCount;
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0); // 清除纹理槽
    }
}