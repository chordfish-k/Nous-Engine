#pragma once

#include "Nous/Core/Base.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Nous {

    class Log
    {
    public:
        static void Init(bool toFile = false);

        static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
        static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
    };
}

template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
    return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
    return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion)
{
    return os << glm::to_string(quaternion);
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

