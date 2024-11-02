#include "pch.h"
#include "Console.h"

#include "Nous/UI/UI.h"
#include "Nous/Core/Log.h"

namespace Nous
{
    std::deque<Console::Line> Console::m_ConsoleLines;
    std::mutex Console::m_Mutex;
    uint32_t Console::m_MaxLines = 9999;
    bool Console::m_AfterAddLog = false;

    Console::Console()
    {
    }

    void Console::Clear()
    {
        std::scoped_lock<std::mutex> lock(m_Mutex);
        m_ConsoleLines.clear();
    }

    void Console::Init()
    {
        auto console_sink = std::make_shared<ConsoleSink<std::mutex>>();

        auto& loggerCore = Log::GetCoreLogger();
        loggerCore = std::make_shared<spdlog::logger>("NOUS", console_sink);
        spdlog::set_default_logger(loggerCore);
        spdlog::set_level(spdlog::level::trace);

        auto& loggerClient = Log::GetClientLogger();
        loggerClient = std::make_shared<spdlog::logger>("APP", console_sink);
        spdlog::set_default_logger(loggerClient);
        spdlog::set_level(spdlog::level::trace);
    }

    void Console::AddLog(const Console::Line& log)
    {
        std::scoped_lock<std::mutex> lock(m_Mutex);
        m_ConsoleLines.push_back(log);

        if (m_ConsoleLines.size() > m_MaxLines)
        {
            m_ConsoleLines.pop_front();
        }

        m_AfterAddLog = true;
    }

    void Console::SetMaxLines(uint32_t lines)
    {
        m_MaxLines = lines;
    }

    template<typename T>
    void ConsoleSink<T>::sink_it_(const spdlog::details::log_msg& msg) {
        // 将日志消息格式化
        auto timeT = std::chrono::system_clock::to_time_t(msg.time);
        auto localTime = *std::localtime(&timeT);
        auto name = *msg.logger_name;
        char buffer[16];
        std::string text = fmt::to_string(msg.payload);
        //std::string formatted = fmt::format("{0} [{1}] {2}: {3}", time, spdlog::level::level_names[msg.level], name, msg);
        
        Console::Line line;

        auto begin = text.find_last_of("{{");
        auto end = text.find_last_of("}}");
        if (begin != std::string::npos && end != std::string::npos && begin < end && end == text.size() - 1)
        {
            auto sp = text.find_last_of(",");
            if (sp != std::string::npos && sp > begin && sp < end)
            {
                line.Filepath = text.substr(begin + 1, sp - begin - 1);
                line.LineNum = std::atoi(text.substr(sp + 1, end - sp - 1).c_str());
                line.Msg = text.substr(0, begin);
            }
        }
        else
        {
            line.Msg = text;
        }
        line.Level = msg.level;
        line.Timestamp = localTime;
        line.LoggerName = name;
        
        Console::AddLog(line);
    }
}