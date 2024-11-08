#include "pch.h"
#include "WinsWindow.h"

#include "Nous/Event/ApplicationEvent.h"
#include "Nous/Event/KeyEvent.h"
#include "Nous/Event/MouseEvent.h"
#include "Nous/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLContext.h"

namespace Nous {

    // 窗口数量
    static int s_GLFWWindowCount = 0;

    static void GLFWErrorCallback(int error, const char* desc)
    {
        NS_CORE_ERROR("GLFW Error ({0}): {1}", error, desc);
    }

    WinsWindow::WinsWindow(const WindowProps& props)
    {
        NS_PROFILE_FUNCTION();

        Init(props);
    }

    WinsWindow::~WinsWindow()
    {
        NS_PROFILE_FUNCTION();

        Shutdown();
    }

    void WinsWindow::Init(const WindowProps& props)
    {
        NS_PROFILE_FUNCTION();

        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        NS_CORE_INFO("创建窗口: {0} ({1}, {2})", props.Title, props.Width, props.Height);

        if (s_GLFWWindowCount == 0)
        {
            NS_PROFILE_SCOPE("glfwInit");
            int success = glfwInit();
            NS_CORE_ASSERT(success, "Could not intialize GLFW!");
            glfwSetErrorCallback(GLFWErrorCallback);
        }

        {
            NS_PROFILE_SCOPE("glfwCreateWindow");
#if defined(NS_DEBUG)
            if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
            m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), NULL, NULL);
            ++s_GLFWWindowCount;
        }
        // 初始化上下文
        m_Context = new OpenGLContext(m_Window);
        m_Context->Init();

        glfwSetWindowUserPointer(m_Window, &m_Data);
        SetVSync(true);

        // 窗口居中
        {
            // 获取屏幕分辨率
            int screenWidth, screenHeight;
            glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), NULL, NULL, &screenWidth, &screenHeight);

            // 获取窗口大小
            int windowWidth, windowHeight;
            glfwGetWindowSize(m_Window, &windowWidth, &windowHeight);

            // 计算窗口居中位置
            int xPos = (screenWidth - windowWidth) / 2;
            int yPos = (screenHeight - windowHeight) / 2;

            // 设置窗口位置
            glfwSetWindowPos(m_Window, xPos, yPos);
        }
        

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
                    KeyPressedEvent event(key, true);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            KeyTypedEvent event(static_cast<KeyCode>(keycode));
            data.EventCallback(event);
        });


        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* win, int button, int action, int mods)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(win);

            switch (action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(static_cast<MouseCode>(button));
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(static_cast<MouseCode>(button));
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

        glfwSetDropCallback(m_Window, [](GLFWwindow* win, int pathCount, const char* paths[])
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(win);

            std::vector<std::filesystem::path> filepaths(pathCount);
            for (int i = 0; i < pathCount; i++)
                filepaths[i] = std::filesystem::u8path(paths[i]);
            

            WindowDropEvent event(std::move(filepaths));
            data.EventCallback(event);
        });
    }

    void WinsWindow::SetVSync(bool enabled)
    {
        NS_PROFILE_FUNCTION();

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
        NS_PROFILE_FUNCTION();

        glfwPollEvents();
        m_Context->SwapBuffers();
    }

    void WinsWindow::Shutdown()
    {
        NS_PROFILE_FUNCTION();
        glfwDestroyWindow(m_Window);

        if (--s_GLFWWindowCount == 0)
        {
            glfwTerminate();
        }
    }
}