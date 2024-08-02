#pragma once

#include "Nous/Core/Base.h"
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
        Application(const std::string& name = "Nous App");
        virtual ~Application();

        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);

        void Close();

        Window& GetWindow() { return *m_Window; }

        ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
        static Application& Get() { return *s_Instance; }

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
