#include "pch.h"
#include "Application.h"

#include "Nous/Event/ApplicationEvent.h"
#include "Nous/Log.h"

namespace Nous {

    Application::Application()
    {

    }

    Application::~Application()
    {

    }

    void Application::Run()
    {
        WindowResizeEvent e(1280, 720);
        NS_TRACE(e);

        while (true);
    }

}