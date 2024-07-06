#include "pch.h"
#include "WinsWindow.h"

#include "Nous/Event/ApplicationEvent.h"
#include "Nous/Event/KeyEvent.h"
#include "Nous/Event/MouseButtonEvent.h"

#include <glad/glad.h>

namespace Nous {

    // 是否已经初始化GLFW
    static bool s_GLFWInitialized = false;

    static void GLFWErrorCallback(int error, const char* desc)
    {
        NS_CORE_ERROR("GLFW Error ({0}): {1}", error, desc);
    }

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
            glfwSetErrorCallback(GLFWErrorCallback);
            s_GLFWInitialized = true;
        }

        m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), NULL, NULL);
        glfwMakeContextCurrent(m_Window);
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        NS_CORE_ASSERT(status, "Failed to initialze Glad!");

        glfwSetWindowUserPointer(m_Window, &m_Data);
        SetVSync(true);

        // 设置GLFW回调
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* win, int w, int h)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(win);
            data.Width = w;
            data.Height = h;

            WindowResizeEvent event(w, h);
            data.EventCallback(event);
        });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* win)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(win);
            WindowCloseEvent event;
            data.EventCallback(event);
        });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* win, int key, int scancode, int action, int mods)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(win);

            switch (action)
            {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event(key, 0);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(key);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event(key, 1);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* win, int button, int action, int mods)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(win);

            switch (action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(button);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(button);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* win, double xOffset, double yOffset)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(win);

            MouseScrolledEvent event((float)xOffset, (float)yOffset);
            data.EventCallback(event);
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* win, double xpos, double ypos)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(win);

            MouseMovedEvent event((float)xpos, (float)ypos);
            data.EventCallback(event);
        });
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