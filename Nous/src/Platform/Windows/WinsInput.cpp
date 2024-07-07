#include "pch.h"
#include "WinsInput.h"

#include "Nous/Application.h"
#include <GLFW/glfw3.h>

namespace Nous {

    Input* Input::s_Instance = new WinsInput();

    bool WinsInput::IsKeyPressedImpl(int keyCode)
    {
        auto window = static_cast<GLFWwindow *>(Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetKey(window, keyCode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool WinsInput::IsMouseButtonPressedImpl(int button)
    {
        auto window = static_cast<GLFWwindow *>(Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    }

    std::pair<float, float> WinsInput::GetMousePosImpl()
    {
        auto window = static_cast<GLFWwindow *>(Application::Get().GetWindow().GetNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return { (float) xpos, (float) ypos };
    }

    float WinsInput::GetMouseXImpl()
    {
        // c++17 结构化绑定
        auto [x, _] = GetMousePosImpl();
        return x;
    }

    float WinsInput::GetMouseYImpl()
    {
        auto [_, y] = GetMousePosImpl();
        return y;
    }

}

