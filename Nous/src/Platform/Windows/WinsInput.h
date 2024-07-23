#pragma once

#include "Nous/Core/Input.h"

namespace Nous {

    class WinsInput : public Input
    {
    protected:
        virtual bool IsKeyPressedImpl(int keyCode) override;

        bool IsMouseButtonPressedImpl(int button) override;

        std::pair<float, float> GetMousePosImpl() override;

        float GetMouseXImpl() override;

        float GetMouseYImpl() override;
    };

}

