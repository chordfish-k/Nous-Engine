#pragma once

#include "Nous/Core/KeyCodes.h"
#include "Nous/Core/MouseCodes.h"

#include <glm/glm.hpp>

namespace Nous {

    class Input
    {
    protected:
        Input() = default;
    public:
        virtual ~Input() = default;

        static bool IsKeyPressed(KeyCode keycode);
        static bool IsMouseButtonPressed(MouseCode button);
        static glm::vec2 GetMousePos();
        static float GetMouseX();
        static float GetMouseY();
    };

}
