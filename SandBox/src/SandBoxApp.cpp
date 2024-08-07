#include "Nous.h"
#include "Nous/Core/EntryPoint.h"

#include "SandBox2D.h"

namespace Nous {

    class Sandbox : public Application
    {
    public:
        Sandbox(ApplicationCommandLineArgs args)
            : Application("Sandbox", args)
        {
//        PushLayer(new ExampleLayer());
            PushLayer(new SandBox2D());
        }

        ~Sandbox()
        {

        }
    };

// 使用Nous的程序入口点，提供一个创建应用的函数
    Application* CreateApplication(ApplicationCommandLineArgs args)
    {
        return new Sandbox(args);
    }

}
