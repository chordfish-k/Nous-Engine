#include "Nous.h"
#include "Nous/Core/EntryPoint.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include "EditorLayer.h"

namespace Nous {

    class NousEditor : public Application
    {
    public:
        NousEditor(ApplicationCommandLineArgs args)
            : Application("Nous Editor", args)
        {
            PushLayer(new EditorLayer());
        }

        ~NousEditor()
        {
        }
    };

// 使用Nous的程序入口点，提供一个创建应用的函数
    Application* CreateApplication(ApplicationCommandLineArgs args)
    {
        return new NousEditor(args);
    }
}
