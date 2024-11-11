﻿#pragma once

#include "Nous/Core/Base.h"
#include "Nous/Core/Window.h"

#include "Nous/Core/LayerStack.h"
#include "Nous/Event/Event.h"
#include "Nous/Event/ApplicationEvent.h"
#include "Nous/ImGui/ImGuiLayer.h"

int main(int argc, char** argv);

namespace Nous {

    struct ApplicationCommandLineArgs
    {
        int Count = 0;
        char** Args = nullptr;

        const char* operator[](int index) const
        {
            NS_CORE_ASSERT(index < Count);
            return Args[index];
        }
    };

    struct ApplicationSpecification
    {
        std::string Name = "Nous Application";
        std::string ImguiConfigFile = "imgui.ini";
        unsigned int Width = 1600;
        unsigned int Height = 900;
        std::string WorkingDirectory;
        ApplicationCommandLineArgs CommandLineArgs;
    };

    // 用于管理游戏主循环
    class Application
    {
    public:
        Application(const ApplicationSpecification& specification);
        virtual ~Application();

        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);

        void Close();

        Window& GetWindow() { return *m_Window; }

        ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
        static Application& Get() { return *s_Instance; }

        const ApplicationSpecification& GetSpecification() const { return m_Specification; }
    
        void SubmitToMainThread(const std::function<void()>& function);
    private:
        void Run();
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);

        void ExecuteMainThreadQueue();
    private:
        ApplicationSpecification m_Specification;
        Scope<Window> m_Window;
        ImGuiLayer* m_ImGuiLayer;
        bool m_Running = true;
        bool m_Minimized = false;
        LayerStack m_LayerStack;
        float m_LastFrameTime = 0.0f;

        std::vector<std::function<void()>> m_MainThreadQueue;
        std::mutex m_MainThreadQueueMutex;
    private:
        static Application* s_Instance;
        friend int ::main(int argc, char** argv);
    };

    // 需要在客户端中定义
    Application* CreateApplication(ApplicationCommandLineArgs args);

}
