#pragma once

#include "Nous/Core/Core.h"
#include "Nous/Core/Window.h"

#include "Nous/Core/LayerStack.h"
#include "Nous/Event/Event.h"
#include "Nous/Event/ApplicationEvent.h"
#include "Nous/ImGui/ImGuiLayer.h"

int main(int argc, char** argv);

namespace Nous {

    // 用于管理游戏主循环
    class NOUS_API Application
    {
    public:
        Application();
        virtual ~Application();

        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);

        inline static Application& Get() { return *s_Instance; }

        inline Window& GetWindow() { return *m_Window; }

    private:
        void Run();
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);

    private:
        Scope<Window> m_Window;
        ImGuiLayer* m_ImGuiLayer;
        bool m_Running = true;
        bool m_Minimized = false;
        LayerStack m_LayerStack;
        float m_lastFrameTime = 0.0f;

        static Application* s_Instance;

        friend int ::main(int argc, char** argv);
    };

    // 需要在客户端中定义
    Application* CreateApplication();

}
