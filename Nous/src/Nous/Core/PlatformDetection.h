#pragma once

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