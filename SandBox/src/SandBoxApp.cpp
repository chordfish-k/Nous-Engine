#include <Nous.h>

class ExampleLayer : public Nous::Layer
{
public:
    ExampleLayer()
            : Layer("Example")
    {

    }

    void OnUpdate() override
    {
        NS_INFO("ExampleLayer::Update");
    }

    void OnEvent(Nous::Event& event) override
    {
        NS_TRACE("{}", event);
    }
};

class Sandbox : public Nous::Application
{
public:
    Sandbox()
    {
        PushLayer(new ExampleLayer());
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