#pragma once

#include "Core.h"

#include "Window.h"
#include "Nous/LayerStack.h"
#include "Nous/Event/Event.h"
#include "Nous/Event/ApplicationEvent.h"

#include "Nous/ImGui/ImGuiLayer.h"

#include "Nous/Renderer/Shader.h"
#include "Nous/Renderer/Buffer.h"

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

        std::unique_ptr<Window> m_Window;
        ImGuiLayer* m_ImGuiLayer;
        bool m_Running = true;
        LayerStack m_LayerStack;

        unsigned int m_VertexArray;
        std::unique_ptr<Shader> m_Shader;
        std::unique_ptr<VertexBuffer> m_VertexBuffer;
        std::unique_ptr<IndexBuffer> m_IndexBuffer;

    private:
        static Application* s_Instance;
    };

    // 需要在客户端中定义
    Application* CreateApplication();

}
