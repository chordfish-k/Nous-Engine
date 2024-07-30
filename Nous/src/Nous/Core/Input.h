#pragma once

#include "Core.h"
#include "Nous/Core/KeyCodes.h"
#include "Nous/Core/MouseCodes.h"

namespace Nous {

    class NOUS_API Input
    {
        // 这里巧妙地解决了静态函数无法重载的问题
    public:
        static bool IsKeyPressed(KeyCode keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
        static bool IsMouseButtonPressed(MouseCode button) { return s_Instance->IsMouseButtonPressedImpl(button); }
        static std::pair<float, float> GetMousePos() { return s_Instance->GetMousePosImpl(); }
        static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
        static float GetMouseY() { return s_Instance->GetMouseYImpl(); }

        static Scope<Input> Create();

    protected:
        virtual bool IsKeyPressedImpl(KeyCode keycode) = 0;
        virtual bool IsMouseButtonPressedImpl(MouseCode button) = 0;
        virtual std::pair<float, float> GetMousePosImpl() = 0;
        virtual float GetMouseXImpl() = 0;
        virtual float GetMouseYImpl() = 0;

    private:
        static Scope<Input> s_Instance;
    };

}
