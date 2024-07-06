#pragma once

#include "Core.h"

#include "Window.h"
#include "Nous/LayerStack.h"
#include "Nous/Event/Event.h"
#include "Nous/Event/ApplicationEvent.h"

namespace Nous {

    // 用于管理游戏主循环
    class NOUS_API Application
    {
    public:
        Application();
        virtual ~Application();

        void Run();

        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PopLayer(Layer* layer);

    private:
        bool OnWindowClose(WindowCloseEvent& e);

        std::unique_ptr<Window> m_Window;
        bool m_Running = true;
        LayerStack m_LayerStack;
    };

    // 需要在客户端中定义
    Application* CreateApplication();

}
