#pragma once

//#include "Nous/Core/Base.h"
#include "Nous/Core/Log.h"
#include <filesystem>

#ifdef NS_ENABLE_ASSERTS
	#define NS_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { NS##type##ERROR(msg, __VA_ARGS__); NS_DEBUGBREAK(); } }
	#define NS_INTERNAL_ASSERT_WITH_MSG(type, check, ...) NS_INTERNAL_ASSERT_IMPL(type, check, "断言失败: {0}", __VA_ARGS__)
	#define NS_INTERNAL_ASSERT_NO_MSG(type, check) NS_INTERNAL_ASSERT_IMPL(type, check, "断言 '{0}' 失败于 {1}:{2}", NS_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

	#define NS_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define NS_INTERNAL_ASSERT_GET_MACRO(...) NS_EXPAND_MACRO( NS_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, NS_INTERNAL_ASSERT_WITH_MSG, NS_INTERNAL_ASSERT_NO_MSG) )

	#define NS_ASSERT(...) NS_EXPAND_MACRO( NS_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define NS_CORE_ASSERT(...) NS_EXPAND_MACRO( NS_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
    #define NS_ASSERT(...)
    #define NS_CORE_ASSERT(...)
#endif
