#include "pch.h"
#include "Application.h"

#include "Log.h"
#include "Input.h"
#include "Nous/Renderer/Renderer.h"

namespace Nous {

    Application* Application::s_Instance = nullptr;

    Application::Application(const std::string& name)
    {
        NS_PROFILE_FUNCTION();

        NS_CORE_ASSERT(!s_Instance, "Application 已经存在!");
        s_Instance = this;

        // 唯一指针，当Application销毁时一并销毁
        m_Window = Scope<Window>(Window::Create(WindowProps(name)));
        m_Window->SetEventCallback(NS_BIND_EVENT_FN(Application::OnEvent));

        Renderer::Init();

        // 创建ImGui层
        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);

    }

    Application::~Application()
    {
        NS_PROFILE_FUNCTION();

        Renderer::Shutdown();
    }

    void Application::OnEvent(Event& e)
    {
        NS_PROFILE_FUNCTION();

        // 处理窗口关闭事件
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(NS_BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(NS_BIND_EVENT_FN(Application::OnWindowResize));

        // 从后往前逐层处理事件，直到这个事件被处理完毕
        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            if (e.Handled)
                break;
            (*it)->OnEvent(e);
        }
    }

    void Application::PushLayer(Layer* layer)
    {
        NS_PROFILE_FUNCTION();

        m_LayerStack.PushLayer(layer);
        layer->OnAttached();
    }

    void Application::PushOverlay(Layer* overlay)
    {
        NS_PROFILE_FUNCTION();

        m_LayerStack.PushOverlay(overlay);
        overlay->OnAttached();
    }

    void Application::Close()
    {
        m_Running = false;
    }

    void Application::Run()
    {
        NS_PROFILE_FUNCTION();

        while (m_Running)
        {
            NS_PROFILE_SCOPE("RunLoop");

            float time = (float) Time::Now();
            Timestep timestep = time - m_lastFrameTime;
            m_lastFrameTime = time;

            if (!m_Minimized)
            {
                {
                    NS_PROFILE_SCOPE("LayerStack OnUpdateRuntime");

                    for (auto* layer: m_LayerStack)
                        layer->OnUpdate(timestep);
                }

                m_ImGuiLayer->Begin();

                {
                    NS_PROFILE_SCOPE("LayerStack OnImGuiRender");

                    for (auto* layer: m_LayerStack)
                        layer->OnImGuiRender();
                }
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
        NS_PROFILE_FUNCTION();

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