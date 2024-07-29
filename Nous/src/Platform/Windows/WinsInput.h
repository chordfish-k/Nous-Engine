#pragma once

#include "Nous/Core/Input.h"

namespace Nous {

    class WinsInput : public Input
    {
    protected:
        virtual bool IsKeyPressedImpl(KeyCode keyCode) override;

        bool IsMouseButtonPressedImpl(MouseCode button) override;

        std::pair<float, float> GetMousePosImpl() override;

        float GetMouseXImpl() override;

        float GetMouseYImpl() override;
    };

}

