#include "pch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Nous {

    OpenGLContext::OpenGLContext(GLFWwindow *windowHandle)
            : windowHandle(windowHandle) {}

    void OpenGLContext::Init()
    {
        NS_PROFILE_FUNCTION();

        glfwMakeContextCurrent(windowHandle);
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        NS_CORE_ASSERT(status, "Failed to initialze Glad!");

        NS_CORE_INFO("OpenGL 信息:");
        NS_CORE_INFO("  厂商: {0}", glGetString(GL_VENDOR));
        NS_CORE_INFO("  渲染器: {0}", glGetString(GL_RENDERER));
        NS_CORE_INFO("  版本: {0}", glGetString(GL_VERSION));

#ifdef NS_ENABLE_ASSERTS
        int versionMajor;
        int versionMinor;
        glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
        glGetIntegerv(GL_MINOR_VERSION, &versionMinor);
        NS_CORE_ASSERT(versionMajor > 4 || (versionMajor == 4 && versionMinor >= 5), "Nous 需要 OpenGL 4.5 或以上版本!");
#endif
    }

    void OpenGLContext::SwapBuffers()
    {
        NS_PROFILE_FUNCTION();

        glfwSwapBuffers(windowHandle);
    }
}