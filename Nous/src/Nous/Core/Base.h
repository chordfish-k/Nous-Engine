﻿#pragma once

#include <memory>

// 基于宏的自动平台检测
#ifdef _WIN32
// Windows x64/x86
    #ifdef _WIN64
// Windows x64
        #ifndef NS_PLATFORM_WINDOWS
            #define NS_PLATFORM_WINDOWS
        #endif
    #else
// Windows x86
        #error "x86 Builds are not supported!"
    #endif
#elif defined(__APPLE__) || defined(__MACH__)
#include <TargetConditionals.h>
/* TARGET_OS_MAC 各个平台都存在
 * 必须以这个顺序检测它们
 * 来保证程序在 MAC 运行
 * 而不是其它 Apple 平台 */
#if TARGET_IPHONE_SIMULATOR == 1
    #error "IOS simulator is not supported!"
#elif TARGET_OS_IPHONE == 1
    #define NS_PLATFORM_IOS
    #error "IOS is not supported!"
#elif TARGET_OS_MAC == 1
    #define NS_PLATFORM_MACOS
    #error "MacOS is not supported!"
#else
    #error "Unknown Apple platform!"
#endif
/* 先检测 __ANDROID__ 再检测 __linux__
* 因为 android 基于 linux */
#elif defined(__ANDROID__)
    #define NS_PLATFORM_ANDROID
    #error "Android is not supported!"
#elif defined(__linux__)
    #define NS_PLATFORM_LINUX
    #error "Linux is not supported!"
#else
/* Unknown compiler/platform */
    #error "Unknown platform!"
#endif


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