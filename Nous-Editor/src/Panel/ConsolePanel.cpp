#include "pch.h"
#include "ConsolePanel.h"

#include "Nous/Core/Console.h"
#include <Nous/UI/UI.h>

namespace Nous
{
    ConsolePanel::ConsolePanel()
    {
        AppEventEmitter::AddObserver(this);
    }

    void ConsolePanel::OnImGuiRender()
    {
        ImGui::Begin("Console");

        if (ImGui::Button("Clear"))
            Clear();

        ImGui::BeginChild("ConsoleOutput");

        auto& lines = Console::GetConsoleLines();

        // 只显示特定范围，减少渲染压力
        int index = -1;
        int scrollY = ImGui::GetScrollY();
        float lineHeight = (ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.y);
        int beginLine = scrollY / lineHeight;
        auto p = ImGui::GetContentRegionAvail().y;
        int endLine = beginLine + ImGui::GetContentRegionAvail().y / lineHeight;
        
        for (const auto& line : lines)
        {
            index++;
            if (index < beginLine || index > endLine + 2)
            {
                ImGui::NewLine();
                continue;
            }

            char timeBuffer[16];
            std::strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &line.Timestamp);
            std::string msg = fmt::format("[{0}] {1}: {2}", timeBuffer, line.LoggerName, line.Msg);
            //std::string msg = fmt::format("[{0}] [{1}] {2}: {3}", timeBuffer, spdlog::level::short_level_names[line.Level], line.LoggerName, line.Msg);
            {
                static ImVec4 colors[] =
                {
                    {1, 1, 1, 1}, // trace
                    {0.5, 0.5, 0.5, 1}, // debug
                    {0, 1, 0, 1}, // info
                    {1, 1, 0, 1}, // warn
                    {1, 0, 0, 1}, // err
                    {1, 0, 0, 1}, // critical
                };
                auto& color = colors[(int)line.Level];
                UI::ScopedStyleColor scopeColor(ImGuiCol_Text, color);
                ImGui::Text(NS_TEXT(msg));
            }

            if (!line.Filepath.empty())
            {
                ImGui::SameLine();

                std::string fileName = std::filesystem::path(line.Filepath).filename().string();
                std::string rightText = fmt::format("{0}:{1}", fileName, line.LineNum);
                float rightTextWidth = ImGui::CalcTextSize(rightText.c_str()).x;
                float availableWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ScrollbarSize;
                ImGui::SameLine(0, availableWidth - rightTextWidth); 
                ImGui::Text(rightText.c_str());

                if (ImGui::IsItemHovered()) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                    ImGui::SetTooltip(line.Filepath.c_str());
                }
            }
        }

        if (Console::IsAfterAddLog())
        {
            ImGui::SetScrollHereY(1.0);
            Console::ResetAfterAddLog();
        }
        ImGui::EndChild();

        ImGui::End();
    }

    void ConsolePanel::Clear()
    {
        Console::Clear();
    }

    void ConsolePanel::OnEditorEvent(AppEvent& e)
    {
        AppEventDispatcher dispatcher(e);
        dispatcher.Dispatch<ConsoleClearEvent>(NS_BIND_EVENT_FN(ConsolePanel::OnConsoleClear));
    }

    void ConsolePanel::OnConsoleClear(const ConsoleClearEvent& e)
    {
        Console::Clear();
    }
}