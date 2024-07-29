#include "Nous.h"
#include "Nous/Core/EntryPoint.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include "SandBox2D.h"

class Sandbox : public Nous::Application
{
public:
    Sandbox()
    {
//        PushLayer(new ExampleLayer());
        PushLayer(new SandBox2D());
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