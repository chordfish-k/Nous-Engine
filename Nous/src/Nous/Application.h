#pragma once

#include "Core.h"
#include "Window.h"

#include "Nous/LayerStack.h"
#include "Nous/Event/Event.h"
#include "Nous/Event/ApplicationEvent.h"
#include "Nous/ImGui/ImGuiLayer.h"

#include "Nous/Renderer/Shader.h"

#include "Nous/Renderer/Buffer.h"
#include "Nous/Renderer/VertexArray.h"
#include "Nous/Renderer/Camera.h"

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

        void PushOverlay(Layer* overlay);

        inline static Application& Get() { return *s_Instance; }

        inline Window& GetWindow() { return *m_Window; }

    private:
        bool OnWindowClose(WindowCloseEvent& e);

    private:
        Scope<Window> m_Window;
        ImGuiLayer* m_ImGuiLayer;
        bool m_Running = true;
        LayerStack m_LayerStack;
        float m_lastFrameTime = 0.0f;

        static Application* s_Instance;
    };

    // 需要在客户端中定义
    Application* CreateApplication();

}
