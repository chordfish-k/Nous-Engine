#pragma once

#include <imgui.h>
#include <vector>
#include <string>

#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/sink.h>

#include "Nous/Core/Log.h"


namespace Nous
{

    class Console
    {
    public:
        Console();

        static void Clear();
        static void Init();
        static void AddLog(const std::string& log);
        void OnImGuiRender();

    public:
        static std::vector<std::string> ConsoleLines;
    };


    template<typename Mutex>
    class ConsoleSink : public spdlog::sinks::base_sink<Mutex>
    {
    public:
        explicit ConsoleSink() {}

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override {
            // 将日志消息格式化
            auto timeT = std::chrono::system_clock::to_time_t(msg.time);
            auto localTime = *std::localtime(&timeT);
            auto name = *msg.logger_name;
            char buffer[16];
            std::strftime(buffer, sizeof(buffer), "[%H:%M:%S]", &localTime);
            std::string time = std::string(buffer);
            std::string formatted = fmt::format("{0} {1}: {2}", time, name, fmt::to_string(msg.payload));

            // 将格式化后的消息加入 Console
            Console::AddLog(fmt::to_string(formatted));
        }

        void flush_() override {}
    };




    
    
}


