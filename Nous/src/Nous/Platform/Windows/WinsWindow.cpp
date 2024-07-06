#include "pch.h"
#include "WinsWindow.h"

namespace Nous {

    // 是否已经初始化GLFW
    static bool s_GLFWInitialized = false;

    // 根据所属平台不同创建不同的实现类对象
    Window* Window::Create(const WindowProps& props)
    {
        return new WinsWindow(props);
    }

    WinsWindow::WinsWindow(const WindowProps& props)
    {
        Init(props);
    }

    WinsWindow::~WinsWindow()
    {
        Shutdown();
    }

    void WinsWindow::Init(const WindowProps& props)
    {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        NS_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

        if (!s_GLFWInitialized)
        {
            int success = glfwInit();
            NS_CORE_ASSERT(success, "Could not intialize GLFW!");

            s_GLFWInitialized = true;
        }

        m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), NULL, NULL);
        glfwMakeContextCurrent(m_Window);
        glfwSetWindowUserPointer(m_Window, &m_Data);
        SetVSync(true);
    }

    void WinsWindow::SetVSync(bool enabled)
    {
        if (enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);
    }

    bool WinsWindow::IsVSync() const
    {
        return m_Data.VSync;
    }

    void WinsWindow::OnUpdate()
    {
        glfwPollEvents();
        glfwSwapBuffers(m_Window);
    }

    void WinsWindow::Shutdown()
    {
        glfwDestroyWindow(m_Window);
    }
}