#pragma once

#include "Nous/Core/Layer.h"
#include "Nous/Event/ApplicationEvent.h"
#include "Nous/Event/MouseEvent.h"
#include "Nous/Event/KeyEvent.h"

namespace Nous {

    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer();

        void OnAttached() override;
        void OnDetached() override;
        void OnEvent(Event& e) override;

        void Begin();
        void End();

        void SetBlockEvent(bool block) { m_BlockEvent = block; }
        bool IsBlockEvent() const { return m_BlockEvent; }

        void SetDarkThemeColor();
    private:
        bool m_BlockEvent = true; // 阻止ImGui的事件
        float m_Time = 0.0f;
    };
}