#pragma once

#include "Nous/Core/Base.h"

#ifdef NS_PLATFORM_WINDOWS

extern Nous::Application* Nous::CreateApplication();

int main(int argc, char** argv)
{
    Nous::Log::Init();

    NS_PROFILE_BEGIN_SESSION("Startup", "Profile-Startup.json");
    auto app = Nous::CreateApplication();
    NS_PROFILE_END_SESSION();

    NS_PROFILE_BEGIN_SESSION("Runtime", "Profile-Runtime.json");
    app->Run();
    NS_PROFILE_END_SESSION();

    NS_PROFILE_BEGIN_SESSION("Shutdown", "Profile-Shutdown.json");
    delete app;
    NS_PROFILE_END_SESSION();
    return 0;
}

#endif // NS_PLATFORM_WINDOWS
