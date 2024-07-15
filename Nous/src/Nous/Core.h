#pragma once

#include <memory>

#ifdef NS_PLATFORM_WINDOWS
#ifdef NS_DYNAMIC_LINK
    #ifdef NS_BUILD_DLL
        #define NOUS_API __declspec(dllexport)
    #else
        #define NOUS_API __declspec(dllimport)
    #endif // NS_BUILD_DLL
#else
    #define NOUS_API
#endif
#else
    #error Support Windows Only!
#endif // NS_PLATFORM_WINDOWS

#ifdef NS_ENABLE_ASSERTS
    #define NS_ASSERT(x, fmt) { if(!(x)) { NS_ERROR(fmt); __debugbreak(); } }
    #define NS_CORE_ASSERT(x, fmt) { if(!(x)) { NS_CORE_ERROR(fmt); __debugbreak(); } }
    #define NS_ASSERT_F(x, fmt, ...) { if(!(x)) { NS_ERROR(fmt, __VA_ARGS__); __debugbreak(); } }
    #define NS_CORE_ASSERT_F(x, fmt, ...) { if(!(x)) { NS_CORE_ERROR(fmt, __VA_ARGS__); __debugbreak(); } }
#else
    #define NS_ASSERT(x, ...)
    #define NS_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define NS_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace Nous {

    template<typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T>
    using Ref = std::shared_ptr<T>;


}