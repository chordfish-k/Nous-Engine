#include "pch.h"
#include "Nous/Core/Input.h"

#include "Nous/Core/Application.h"
#include <GLFW/glfw3.h>

namespace Nous {

    bool Input::IsKeyPressed(KeyCode keyCode)
    {
        auto window = static_cast<GLFWwindow *>(Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetKey(window, static_cast<uint32_t>(keyCode));
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsMouseButtonPressed(MouseCode button)
    {
        auto window = static_cast<GLFWwindow *>(Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetMouseButton(window, static_cast<uint32_t>(button));
        return state == GLFW_PRESS;
    }

    std::pair<float, float> Input::GetMousePos()
    {
        auto window = static_cast<GLFWwindow *>(Application::Get().GetWindow().GetNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return { (float) xpos, (float) ypos };
    }

    float Input::GetMouseX()
    {
        // c++17 结构化绑定
        auto [x, _] = GetMousePos();
        return x;
    }

    float Input::GetMouseY()
    {
        auto [_, y] = GetMousePos();
        return y;
    }

}

