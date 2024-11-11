#include "pch.h"
#include "Application.h"

#include "Nous/Core/Log.h"
#include "Nous/Core/Input.h"
#include "Nous/Script/ScriptEngine.h"
#include "Nous/Renderer/Renderer.h"
#include "Nous/Utils/PlatformUtils.h"

namespace Nous {

    Application* Application::s_Instance = nullptr;

    Application::Application(const ApplicationSpecification& specification)
        : m_Specification(specification)
    {
        NS_PROFILE_FUNCTION();

        NS_CORE_ASSERT(!s_Instance, "Application 已经存在!");
        s_Instance = this;

        // 设置 working directory
        if (!m_Specification.WorkingDirectory.empty())
            std::filesystem::current_path(m_Specification.WorkingDirectory);

        m_Window = Window::Create(WindowProps(m_Specification.Name, m_Specification.Width, m_Specification.Height));
        m_Window->SetEventCallback(NS_BIND_EVENT_FN(Application::OnEvent));

        // 创建ImGui层
        m_ImGuiLayer = new ImGuiLayer(this, m_Specification.ImguiConfigFile);
        PushOverlay(m_ImGuiLayer);

    }

    Application::~Application()
    {
        NS_PROFILE_FUNCTION();
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

    void Application::SubmitToMainThread(const std::function<void()>& function)
    {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

        m_MainThreadQueue.emplace_back(function);
    }

    void Application::Run()
    {
        NS_PROFILE_FUNCTION();

        while (m_Running)
        {
            NS_PROFILE_SCOPE("RunLoop");

            float time = Time::GetTime();
            Timestep timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;

            ExecuteMainThreadQueue();

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
    void Application::ExecuteMainThreadQueue()
    {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

        for (auto& func : m_MainThreadQueue)
            func();

        m_MainThreadQueue.clear();
    }
}