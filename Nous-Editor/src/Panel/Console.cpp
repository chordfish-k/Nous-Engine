#include "ConsolePanel.h"

#include "Nous/UI/UI.h"

namespace Nous
{
    std::vector<std::string> Console::ConsoleLines;

    Console::Console()
    {
        
    }
    void Console::Clear()
    {
        ConsoleLines.clear();
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

    void Console::AddLog(const std::string& log)
    {
        Console::ConsoleLines.push_back(NS_TEXT(log));
    }

    void Console::OnImGuiRender()
    {
        ImGui::Begin("Console");

        if (ImGui::Button("Clear"))
            Clear();

        ImGui::BeginChild("ConsoleOutput");
        for (const auto& line : ConsoleLines)
        {
            ImGui::TextUnformatted(line.c_str());
        }
        ImGui::EndChild();

        ImGui::End();
    }

    Console __s;
}