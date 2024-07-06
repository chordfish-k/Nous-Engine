#include "pch.h"
#include "Application.h"

#include "Nous/Log.h"

#include <glad/glad.h>

namespace Nous {

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

    Application::Application()
    {
        // 唯一指针，当Application销毁时一并销毁
        m_Window = std::unique_ptr<Window>(Window::Create());
        m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
    }

    Application::~Application()
    {

    }

    void Application::OnEvent(Event& e)
    {
        // 处理窗口关闭事件
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

        // 从后往前逐层处理事件，直到这个事件被处理完毕
        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
        {
            (*--it)->OnEvent(e);
            if (e.Handled)
                break;
        }
    }

    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
    }

    void Application::PopLayer(Layer* layer)
    {
        m_LayerStack.PopLayer(layer);
    }

    void Application::Run()
    {
        while (m_Running)
        {
            glClearColor(0.2, 0.2, 0.2, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            for (auto* layer : m_LayerStack)
            {
                layer->OnUpdate();
            }

            m_Window->OnUpdate();
        }
    }

    bool Application::OnWindowClose(WindowCloseEvent& e)
    {
        m_Running = false;
        return true;
    }

}