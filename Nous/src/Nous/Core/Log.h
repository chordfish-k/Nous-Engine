#pragma once
#include "Base.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace Nous {

    class NOUS_API Log
    {
    public:
        static void Init();

        static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
        static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
    };
}

// core log 的宏
#define NS_CORE_TRACE(...)		::Nous::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define NS_CORE_INFO(...)		::Nous::Log::GetCoreLogger()->info(__VA_ARGS__)
#define NS_CORE_WARN(...)		::Nous::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define NS_CORE_ERROR(...)		::Nous::Log::GetCoreLogger()->error(__VA_ARGS__)
#define NS_CORE_CRITICAL(...)		::Nous::Log::GetCoreLogger()->critical(__VA_ARGS__)


// client log 的宏
#define NS_TRACE(...)	::Nous::Log::GetClientLogger()->trace(__VA_ARGS__)
#define NS_INFO(...)		::Nous::Log::GetClientLogger()->info(__VA_ARGS__)
#define NS_WARN(...)		::Nous::Log::GetClientLogger()->warn(__VA_ARGS__)
#define NS_ERROR(...)	::Nous::Log::GetClientLogger()->error(__VA_ARGS__)
#define NS_CRITICAL(...)	::Nous::Log::GetClientLogger()->critical(__VA_ARGS__)

