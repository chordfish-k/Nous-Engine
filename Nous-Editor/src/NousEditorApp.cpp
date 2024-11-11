#include "Nous.h"
#include "Nous/Core/EntryPoint.h"
#include "Nous/Core/Console.h"

#include "Nous/Renderer/Renderer.h"
#include "Nous/Script/ScriptEngine.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include "EditorLayer.h"
#include "EditorLuncherLayer.h"

namespace Nous {

    class NousEditor : public Application
    {
    public:
        NousEditor(const ApplicationSpecification& spec)
            : Application(spec)
        {
            ScriptEngine::Init();
            Renderer::Init();
            PushLayer(new EditorLayer(this, spec));
        }

        ~NousEditor()
        {
            Renderer::Shutdown();
            ScriptEngine::Shutdown();
        }
    };

    class NousEditorLuncher : public Application
    {
    public:
        NousEditorLuncher(const ApplicationSpecification& spec)
            : Application(spec)
        {
            PushLayer(new EditorLuncherLayer(this, spec));
        }
    };

    // 使用Nous的程序入口点，提供一个创建应用的函数
    Application* CreateApplication(ApplicationCommandLineArgs args)
    {
        if (args.Count > 1)
        {
            ApplicationSpecification spec;
            spec.Name = "Nous Editor";
            spec.CommandLineArgs = args;
            spec.ImguiConfigFile = "editor.ini";

            Console::Init();
            return new NousEditor(spec);
        }
        else
        {
            ApplicationSpecification spec;
            spec.Name = "Nous Editor";
            spec.CommandLineArgs = args;

            spec.Width = 300;
            spec.Height = 200;
            spec.ImguiConfigFile = "luncher.ini";

            return new NousEditorLuncher(spec);
        }
        
    }
}
