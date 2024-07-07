#include <Nous.h>
#include "Nous/ImGui/ImGuiLayer.h"

class ExampleLayer : public Nous::Layer
{
public:
    ExampleLayer()
            : Layer("Example")
    {

    }

    void OnUpdate() override
    {
        // 轮询方式
        if (Nous::Input::IsKeyPressed(NS_KEY_TAB))
            NS_TRACE("Tab key is pressed! (poll)");
    }

    void OnEvent(Nous::Event& event) override
    {
        // 事件方式
        if (event.GetEventType() == Nous::EventType::KeyPressed)
        {
            auto& e = (Nous::KeyPressedEvent&)event;
            if (e.GetKeyCode() == NS_KEY_TAB)
                NS_TRACE("Tab key is pressed!");
            NS_TRACE("{}", (char)e.GetKeyCode());
        }
    }
};

class Sandbox : public Nous::Application
{
public:
    Sandbox()
    {
        PushLayer(new ExampleLayer());
        PushOverlay(new Nous::ImGuiLayer());
    }

    ~Sandbox()
    {

    }
};

// 使用Nous的程序入口点，提供一个创建应用的函数
Nous::Application* Nous::CreateApplication()
{
    return new Sandbox();
}