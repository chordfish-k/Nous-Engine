#pragma once

#include <string>
#include <deque>

#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/sink.h>

namespace Nous
{
    class Console
    {
    public:
        struct Line
        {
            spdlog::level::level_enum Level = spdlog::level::trace;
            tm Timestamp = {};
            std::string LoggerName = "";
            std::string Msg = "";
            std::string Filepath = "";
            int LineNum = 0;
        };
    public:
        Console();

        static void Init();

        static void Clear();
        static void AddLog(const Line& log);
        static void SetMaxLines(uint32_t lines);
        static uint32_t GetMaxLines() { return m_MaxLines; }
        static std::deque<Line>& GetConsoleLines() { return m_ConsoleLines; }

        static bool IsAfterAddLog() { return m_AfterAddLog; }
        static bool IsInited() { return m_IsInited; }
        static void ResetAfterAddLog() { m_AfterAddLog = false; }

    private:
        static bool m_IsInited;
        static std::mutex m_Mutex;
        static std::deque<Line> m_ConsoleLines;
        static uint32_t m_MaxLines;
        static bool m_AfterAddLog;
    };


    template<typename Mutex>
    class ConsoleSink : public spdlog::sinks::base_sink<Mutex>
    {
    public:
        explicit ConsoleSink() {}
    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override;
        void flush_() override {}
    };
}