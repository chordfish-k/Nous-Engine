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
    }

    void OpenGLContext::SwapBuffers()
    {
        glfwSwapBuffers(windowHandle);
    }
}