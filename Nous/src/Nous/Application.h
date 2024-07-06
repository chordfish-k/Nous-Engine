#pragma once

#include "Core.h"
#include "Event/Event.h"
#include "Window.h"

namespace Nous {

    class NOUS_API Application {
    public:
        Application();

        virtual ~Application();

        void Run();

    private:
        std::unique_ptr<Window> m_Window;
        bool m_Running = true;
    };

    // 需要在客户端中定义
    Application *CreateApplication();

}
