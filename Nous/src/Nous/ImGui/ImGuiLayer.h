#pragma once

#include "Nous/Core/Layer.h"
#include "Nous/Event/ApplicationEvent.h"
#include "Nous/Event/MouseEvent.h"
#include "Nous/Event/KeyEvent.h"

namespace Nous {

    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer(Application* application, const std::string& configFilepath = "imgui.ini");
        ~ImGuiLayer();

        void OnAttached() override;
        void OnDetached() override;
        void OnEvent(Event& e) override;

        void Begin();
        void End();

        // 设置是否阻止ImGui原本键鼠事件
        void SetBlockEvent(bool block) { m_BlockEvent = block; }
        bool IsBlockEvent() const { return m_BlockEvent; }

        void SetDarkThemeColor();

        uint32_t GetActiveWidgetID() const;
    private:
        bool m_BlockEvent = true; // 阻止ImGui的事件
        std::string m_ConfigFilepath;
    };
}