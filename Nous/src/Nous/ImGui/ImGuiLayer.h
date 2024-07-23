#pragma once

#include "Nous/Core/Layer.h"
#include "Nous/Event/ApplicationEvent.h"
#include "Nous/Event/MouseButtonEvent.h"
#include "Nous/Event/KeyEvent.h"

namespace Nous {

    class NOUS_API ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer();

        void OnAttached() override;
        void OnDetached() override;
        void OnImGuiRender() override;

        void Begin();
        void End();
    private:
        float m_Time = 0.0f;
    };
}