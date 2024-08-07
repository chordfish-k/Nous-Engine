#pragma once

#include "Nous/Core/Base.h"
#include "Nous/Core/Application.h"

#ifdef NS_PLATFORM_WINDOWS

extern Nous::Application* Nous::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
    Nous::Log::Init();

    NS_PROFILE_BEGIN_SESSION("Startup", "Profile-Startup.json");
    auto app = Nous::CreateApplication({ argc, argv });
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
