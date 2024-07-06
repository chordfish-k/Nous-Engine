﻿#pragma once

#include "Application.h"
#include <iostream>

#ifdef NS_PLATFORM_WINDOWS

extern Nous::Application* Nous::CreateApplication();

int main(int argc, char** argv)
{
    Nous::Log::Init();
    NS_CORE_WARN("初始化Log");
    int a = 5;
    NS_INFO("Hello! Ver={0}", a);

    auto app = Nous::CreateApplication();
    app->Run();
    delete app;

    return 0;
}

#endif // NS_PLATFORM_WINDOWS
