#pragma once

#include "Core.h"
#include "Nous/Event/Event.h"
#include "Nous/Event/ApplicationEvent.h"
#include "Window.h"

namespace Nous {

    class NOUS_API Application
    {
    public:
        Application();
        virtual ~Application();

        void Run();

        void OnEvent(Event& e);
    private:
        bool OnWindowClose(WindowCloseEvent& e);

        std::unique_ptr<Window> m_Window;
        bool m_Running = true;
    };

    // 需要在客户端中定义
    Application* CreateApplication();

}
