#include "pch.h"
#include "Nous/Core/Input.h"

#ifdef NS_PLATFORM_WINDOWS
    #include "Platform/Windows/WinsInput.h"
#endif

namespace Nous {

    Scope<Input> Input::s_Instance = Input::Create();

    Scope<Input> Input::Create()
    {
#ifdef NS_PLATFORM_WINDOWS
        return CreateScope<WinsInput>();
#else
        NS_CORE_ASSERT(false, "未知的平台定义!");
        return nullptr;
#endif
    }
}