#include "Nous.h"
#include "Nous/Core/EntryPoint.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include "EditorLayer.h"

namespace Nous {

    class NousEditor : public Application
    {
    public:
        NousEditor(const ApplicationSpecification& spec)
            : Application(spec)
        {
            PushLayer(new EditorLayer());
        }
    };

// 使用Nous的程序入口点，提供一个创建应用的函数
    Application* CreateApplication(ApplicationCommandLineArgs args)
    {
        ApplicationSpecification spec;
        spec.Name = "Nous Editor";
        spec.CommandLineArgs = args;
        Console::Init();
        return new NousEditor(spec);
    }
}
