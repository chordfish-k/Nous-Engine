#include "pch.h"
#include "Application.h"

#include "Nous/Log.h"
#include "Nous/Input.h"
#include "Nous/Renderer/Renderer.h"

namespace Nous {

    Application* Application::s_Instance = nullptr;

    Application::Application()
    {
        NS_CORE_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;

        // 唯一指针，当Application销毁时一并销毁
        m_Window = Scope<Window>(Window::Create());
        m_Window->SetEventCallback(NS_BIND_EVENT_FN(Application::OnEvent));

        Renderer::Init();

        // 创建ImGui层
        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);

    }

    Application::~Application()
    {

    }

    void Application::OnEvent(Event& e)
    {
        // 处理窗口关闭事件
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(NS_BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(NS_BIND_EVENT_FN(Application::OnWindowResize));

        // 从后往前逐层处理事件，直到这个事件被处理完毕
        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
        {
            (*--it)->OnEvent(e);
            if (e.Handled)
                break;
        }
    }

    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
        layer->OnAttached();
    }

    void Application::PushOverlay(Layer* overlay)
    {
        m_LayerStack.PushOverlay(overlay);
        overlay->OnAttached();
    }

    void Application::Run()
    {
        while (m_Running)
        {
            float time = (float) Time::Now();
            Timestep timestep = time - m_lastFrameTime;
            m_lastFrameTime = time;

            if (!m_Minimized)
            {
                for (auto* layer: m_LayerStack)
                    layer->OnUpdate(timestep);

                m_ImGuiLayer->Begin();
                for (auto* layer: m_LayerStack)
                    layer->OnImGuiRender();
                m_ImGuiLayer->End();
            }

            m_Window->OnUpdate();
        }
    }

    bool Application::OnWindowClose(WindowCloseEvent& e)
    {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& e)
    {
        if (e.GetWidth() == 0 || e.GetHeight() == 0)
        {
            m_Minimized = true;
            return false;
        }

        m_Minimized = false;
        Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

        return false;
    }
}