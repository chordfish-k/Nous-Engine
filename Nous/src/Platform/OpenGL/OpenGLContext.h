#pragma once

#include "Nous/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Nous {

    /**
     * 封装的OpenGL的渲染API
     */
    class OpenGLContext : public GraphicsContext
    {
    public:
        explicit OpenGLContext(GLFWwindow *windowHandle);

        virtual void Init() override;
        virtual void SwapBuffers() override;

    private:
        GLFWwindow* windowHandle;
    };
}

