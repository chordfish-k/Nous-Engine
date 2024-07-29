#pragma once
#pragma once

namespace Nous
{
    typedef enum class MouseCode : uint16_t
    {
        // 引用自 glfw3.h
        Button0                = 0,
        Button1                = 1,
        Button2                = 2,
        Button3                = 3,
        Button4                = 4,
        Button5                = 5,
        Button6                = 6,
        Button7                = 7,

        ButtonLast             = Button7,
        ButtonLeft             = Button0,
        ButtonRight            = Button1,
        ButtonMiddle           = Button2
    } Mouse;

    inline std::ostream& operator<<(std::ostream& os, MouseCode mouseCode)
    {
        os << static_cast<int32_t>(mouseCode);
        return os;
    }
}

#define NS_MOUSE_BUTTON_0      ::Nous::Mouse::Button0
#define NS_MOUSE_BUTTON_1      ::Nous::Mouse::Button1
#define NS_MOUSE_BUTTON_2      ::Nous::Mouse::Button2
#define NS_MOUSE_BUTTON_3      ::Nous::Mouse::Button3
#define NS_MOUSE_BUTTON_4      ::Nous::Mouse::Button4
#define NS_MOUSE_BUTTON_5      ::Nous::Mouse::Button5
#define NS_MOUSE_BUTTON_6      ::Nous::Mouse::Button6
#define NS_MOUSE_BUTTON_7      ::Nous::Mouse::Button7
#define NS_MOUSE_BUTTON_LAST   ::Nous::Mouse::ButtonLast
#define NS_MOUSE_BUTTON_LEFT   ::Nous::Mouse::ButtonLeft
#define NS_MOUSE_BUTTON_RIGHT  ::Nous::Mouse::ButtonRight
#define NS_MOUSE_BUTTON_MIDDLE ::Nous::Mouse::ButtonMiddle
