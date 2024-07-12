#include "pch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Nous {

    OpenGLContext::OpenGLContext(GLFWwindow *windowHandle)
            : windowHandle(windowHandle) {}

    void OpenGLContext::Init()
    {
        glfwMakeContextCurrent(windowHandle);
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        NS_CORE_ASSERT(status, "Failed to initialze Glad!");

        NS_CORE_INFO("OpenGL 信息:");
        NS_CORE_INFO("  厂商: {0}", glGetString(GL_VENDOR));
        NS_CORE_INFO("  渲染器: {0}", glGetString(GL_RENDERER));
        NS_CORE_INFO("  版本: {0}", glGetString(GL_VERSION));
    }

    void OpenGLContext::SwapBuffers()
    {
        glfwSwapBuffers(windowHandle);
    }
}