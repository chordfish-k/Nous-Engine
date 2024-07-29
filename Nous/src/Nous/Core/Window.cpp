#include "pch.h"
#include "Nous/Core/Window.h"

#ifdef NS_PLATFORM_WINDOWS
    #include "Platform/Windows/WinsWindow.h"
#endif

namespace Nous {

    Scope<Window> Window::Create(const WindowProps& props)
    {
#ifdef NS_PLATFORM_WINDOWS
        return CreateScope<WinsWindow>(props);
#else
        NS_CORE_ASSERT(false, "Unknown platform!");
        return nullptr;
#endif
    }
}