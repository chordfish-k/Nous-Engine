#pragma once

#include "Core.h"
#include "Event/Event.h"

namespace Nous {

    class NOUS_API Application
            {
                    public:
                    Application();
                    virtual ~Application();

                    void Run();
            };

    // 需要在客户端中定义
    Application* CreateApplication();

}
