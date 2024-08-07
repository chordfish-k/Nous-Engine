#pragma once

#include "Base.h"
#include "Nous/Core/KeyCodes.h"
#include "Nous/Core/MouseCodes.h"

namespace Nous {

    class Input
    {
    protected:
        Input() = default;
    public:
        virtual ~Input() = default;

        static bool IsKeyPressed(KeyCode keycode);
        static bool IsMouseButtonPressed(MouseCode button);
        static std::pair<float, float> GetMousePos();
        static float GetMouseX();
        static float GetMouseY();
    };

}
