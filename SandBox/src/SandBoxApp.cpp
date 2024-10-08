﻿#include "Nous.h"
#include "Nous/Core/EntryPoint.h"

#include "SandBox2D.h"

namespace Nous {

    class Sandbox : public Application
    {
    public:
        Sandbox(const ApplicationSpecification& specification)
            : Application(specification)
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
        ApplicationSpecification spec;
        spec.Name = "Sandbox";
        spec.WorkingDirectory = "../Nous-Editor";
        spec.CommandLineArgs = args;

        return new Sandbox(spec);
    }

}
