#include "pch.h"
#include "Application.h"

#include "Nous/Event/ApplicationEvent.h"

#include <GLFW/glfw3.h>

namespace Nous {

    Application::Application()
    {
        // 唯一指针，当Application销毁时一并销毁
        m_Window = std::unique_ptr<Window>(Window::Create());
    }

    Application::~Application()
    {

    }

    void Application::Run()
    {
        while (m_Running)
        {
            glClearColor(0.2, 0.2, 0.2, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            m_Window->OnUpdate();
        }
    }

}