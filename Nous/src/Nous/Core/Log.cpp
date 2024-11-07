#include "pch.h"
#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Nous {

    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
    std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

    void Log::Init(bool toFile)
    {
        if (toFile)
        {
            // 输出到文件尾
            std::vector<spdlog::sink_ptr> logSinks;
            logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
            logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("log.txt", true));

            logSinks[0]->set_pattern("%^[%T] %n: %v%$");
            logSinks[1]->set_pattern("[%T] [%l] %n: %v");

            s_CoreLogger = std::make_shared<spdlog::logger>("NOUS", begin(logSinks), end(logSinks));
            spdlog::register_logger(s_CoreLogger);
            s_CoreLogger->set_level(spdlog::level::trace);
            s_CoreLogger->flush_on(spdlog::level::trace);

            s_ClientLogger = std::make_shared<spdlog::logger>("APP ", begin(logSinks), end(logSinks));
            spdlog::register_logger(s_ClientLogger);
            s_ClientLogger->set_level(spdlog::level::trace);
            s_ClientLogger->flush_on(spdlog::level::trace);
        }
        else
        {
            spdlog::set_pattern("%^[%T] [%L] %n: %v%$");
            s_CoreLogger = spdlog::stdout_color_mt("NOUS");
            s_CoreLogger->set_level(spdlog::level::trace);

            s_ClientLogger = spdlog::stdout_color_mt("APP");
            s_ClientLogger->set_level(spdlog::level::trace);
        }
    }

}
