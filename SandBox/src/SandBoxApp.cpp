#include "Nous.h"
#include "Nous/Core/EntryPoint.h"

#include "Nous/Renderer/Renderer.h"
#include "Nous/Script/ScriptEngine.h"

#include "SandBox2D.h"

namespace Nous {

    class Sandbox : public Application
    {
    public:
        Sandbox(const ApplicationSpecification& specification)
            : Application(specification)
        {
            ScriptEngine::Init();
            Renderer::Init();
            PushLayer(new SandBox2D(this, specification));
        }

        ~Sandbox()
        {
            Renderer::Shutdown();
            ScriptEngine::Shutdown();
        }
    };

// 使用Nous的程序入口点，提供一个创建应用的函数
    Application* CreateApplication(ApplicationCommandLineArgs args)
    {
        ApplicationSpecification spec;
        spec.Name = "Sandbox";
#if NS_RELEASE
        spec.WorkingDirectory = ".";
#endif
#if NS_DEBUG
        spec.WorkingDirectory = "../Nous-Editor";
#endif
        spec.CommandLineArgs = args;

        return new Sandbox(spec);
    }

}
