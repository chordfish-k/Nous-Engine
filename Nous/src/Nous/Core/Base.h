#pragma once

#include "Nous/Core/PlatformDetection.h"

#include <memory>

// DLL 支持
#ifdef NS_PLATFORM_WINDOWS
    #if NS_DYNAMIC_LINK
#ifdef NS_BUILD_DLL
    #define NOUS_API __declspec(dllexport)
#else
    #define NOUS_API __declspec(dllimport)
#endif
    #else
        #define NOUS_API
    #endif
#else
    #error NOUS only supports Windows!
#endif

#ifdef NS_DEBUG
    #if defined(NS_PLATFORM_WINDOWS)
        #define NS_DEBUGBREAK() __debugbreak()
    #elif defined(NS_PLATFORM_LINUX)
        #include <signal.h>
        #define NS_DEBUGBREAK() raise(SIGTRAP)
    #else
        #error "Platform doesn't support debugbreak yet!"
    #endif
    #define NS_ENABLE_ASSERTS
#else
    #define NS_DEBUGBREAK()
#endif

#define NS_EXPAND_MACRO(x) x
#define NS_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define NS_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace Nous {

    template<typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T>
    using Ref = std::shared_ptr<T>;

    template<typename T, typename ... Args>
    constexpr Ref<T> CreateRef(Args&& ... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename ... Args>
    constexpr Scope<T> CreateScope(Args&& ... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
}

#include "Nous/Core/Assert.h"