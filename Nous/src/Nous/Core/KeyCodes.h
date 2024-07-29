#pragma once

#pragma once

namespace Nous
{
    typedef enum class KeyCode : uint16_t
    {
        // 引用自 glfw3.h
        Space               = 32,
        Apostrophe          = 39, /* ' */
        Comma               = 44, /* , */
        Minus               = 45, /* - */
        Period              = 46, /* . */
        Slash               = 47, /* / */

        D0                  = 48, /* 0 */
        D1                  = 49, /* 1 */
        D2                  = 50, /* 2 */
        D3                  = 51, /* 3 */
        D4                  = 52, /* 4 */
        D5                  = 53, /* 5 */
        D6                  = 54, /* 6 */
        D7                  = 55, /* 7 */
        D8                  = 56, /* 8 */
        D9                  = 57, /* 9 */

        Semicolon           = 59, /* ; */
        Equal               = 61, /* = */

        A                   = 65,
        B                   = 66,
        C                   = 67,
        D                   = 68,
        E                   = 69,
        F                   = 70,
        G                   = 71,
        H                   = 72,
        I                   = 73,
        J                   = 74,
        K                   = 75,
        L                   = 76,
        M                   = 77,
        N                   = 78,
        O                   = 79,
        P                   = 80,
        Q                   = 81,
        R                   = 82,
        S                   = 83,
        T                   = 84,
        U                   = 85,
        V                   = 86,
        W                   = 87,
        X                   = 88,
        Y                   = 89,
        Z                   = 90,

        LeftBracket         = 91,  /* [ */
        Backslash           = 92,  /* \ */
        RightBracket        = 93,  /* ] */
        GraveAccent         = 96,  /* ` */

        World1              = 161, /* non-US #1 */
        World2              = 162, /* non-US #2 */

        /* Function keys */
        Escape              = 256,
        Enter               = 257,
        Tab                 = 258,
        Backspace           = 259,
        Insert              = 260,
        Delete              = 261,
        Right               = 262,
        Left                = 263,
        Down                = 264,
        Up                  = 265,
        PageUp              = 266,
        PageDown            = 267,
        Home                = 268,
        End                 = 269,
        CapsLock            = 280,
        ScrollLock          = 281,
        NumLock             = 282,
        PrintScreen         = 283,
        Pause               = 284,
        F1                  = 290,
        F2                  = 291,
        F3                  = 292,
        F4                  = 293,
        F5                  = 294,
        F6                  = 295,
        F7                  = 296,
        F8                  = 297,
        F9                  = 298,
        F10                 = 299,
        F11                 = 300,
        F12                 = 301,
        F13                 = 302,
        F14                 = 303,
        F15                 = 304,
        F16                 = 305,
        F17                 = 306,
        F18                 = 307,
        F19                 = 308,
        F20                 = 309,
        F21                 = 310,
        F22                 = 311,
        F23                 = 312,
        F24                 = 313,
        F25                 = 314,

        /* Keypad */
        KP0                 = 320,
        KP1                 = 321,
        KP2                 = 322,
        KP3                 = 323,
        KP4                 = 324,
        KP5                 = 325,
        KP6                 = 326,
        KP7                 = 327,
        KP8                 = 328,
        KP9                 = 329,
        KPDecimal           = 330,
        KPDivide            = 331,
        KPMultiply          = 332,
        KPSubtract          = 333,
        KPAdd               = 334,
        KPEnter             = 335,
        KPEqual             = 336,

        LeftShift           = 340,
        LeftControl         = 341,
        LeftAlt             = 342,
        LeftSuper           = 343,
        RightShift          = 344,
        RightControl        = 345,
        RightAlt            = 346,
        RightSuper          = 347,
        Menu                = 348
    } Key;

    inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
    {
        os << static_cast<int32_t>(keyCode);
        return os;
    }
}

// 引用自glfw3.h
#define NS_KEY_SPACE           ::Nous::Key::Space
#define NS_KEY_APOSTROPHE      ::Nous::Key::Apostrophe    /* ' */
#define NS_KEY_COMMA           ::Nous::Key::Comma         /* , */
#define NS_KEY_MINUS           ::Nous::Key::Minus         /* - */
#define NS_KEY_PERIOD          ::Nous::Key::Period        /* . */
#define NS_KEY_SLASH           ::Nous::Key::Slash         /* / */
#define NS_KEY_0               ::Nous::Key::D0
#define NS_KEY_1               ::Nous::Key::D1
#define NS_KEY_2               ::Nous::Key::D2
#define NS_KEY_3               ::Nous::Key::D3
#define NS_KEY_4               ::Nous::Key::D4
#define NS_KEY_5               ::Nous::Key::D5
#define NS_KEY_6               ::Nous::Key::D6
#define NS_KEY_7               ::Nous::Key::D7
#define NS_KEY_8               ::Nous::Key::D8
#define NS_KEY_9               ::Nous::Key::D9
#define NS_KEY_SEMICOLON       ::Nous::Key::Semicolon     /* ; */
#define NS_KEY_EQUAL           ::Nous::Key::Equal         /* = */
#define NS_KEY_A               ::Nous::Key::A
#define NS_KEY_B               ::Nous::Key::B
#define NS_KEY_C               ::Nous::Key::C
#define NS_KEY_D               ::Nous::Key::D
#define NS_KEY_E               ::Nous::Key::E
#define NS_KEY_F               ::Nous::Key::F
#define NS_KEY_G               ::Nous::Key::G
#define NS_KEY_H               ::Nous::Key::H
#define NS_KEY_I               ::Nous::Key::I
#define NS_KEY_J               ::Nous::Key::J
#define NS_KEY_K               ::Nous::Key::K
#define NS_KEY_L               ::Nous::Key::L
#define NS_KEY_M               ::Nous::Key::M
#define NS_KEY_N               ::Nous::Key::N
#define NS_KEY_O               ::Nous::Key::O
#define NS_KEY_P               ::Nous::Key::P
#define NS_KEY_Q               ::Nous::Key::Q
#define NS_KEY_R               ::Nous::Key::R
#define NS_KEY_S               ::Nous::Key::S
#define NS_KEY_T               ::Nous::Key::T
#define NS_KEY_U               ::Nous::Key::U
#define NS_KEY_V               ::Nous::Key::V
#define NS_KEY_W               ::Nous::Key::W
#define NS_KEY_X               ::Nous::Key::X
#define NS_KEY_Y               ::Nous::Key::Y
#define NS_KEY_Z               ::Nous::Key::Z
#define NS_KEY_LEFT_BRACKET    ::Nous::Key::LeftBracket   /* [ */
#define NS_KEY_BACKSLASH       ::Nous::Key::Backslash     /* \ */
#define NS_KEY_RIGHT_BRACKET   ::Nous::Key::RightBracket  /* ] */
#define NS_KEY_GRAVE_ACCENT    ::Nous::Key::GraveAccent   /* ` */
#define NS_KEY_WORLD_1         ::Nous::Key::World1        /* non-US #1 */
#define NS_KEY_WORLD_2         ::Nous::Key::World2        /* non-US #2 */

/* Function keys */
#define NS_KEY_ESCAPE          ::Nous::Key::Escape
#define NS_KEY_ENTER           ::Nous::Key::Enter
#define NS_KEY_TAB             ::Nous::Key::Tab
#define NS_KEY_BACKSPACE       ::Nous::Key::Backspace
#define NS_KEY_INSERT          ::Nous::Key::Insert
#define NS_KEY_DELETE          ::Nous::Key::Delete
#define NS_KEY_RIGHT           ::Nous::Key::Right
#define NS_KEY_LEFT            ::Nous::Key::Left
#define NS_KEY_DOWN            ::Nous::Key::Down
#define NS_KEY_UP              ::Nous::Key::Up
#define NS_KEY_PAGE_UP         ::Nous::Key::PageUp
#define NS_KEY_PAGE_DOWN       ::Nous::Key::PageDown
#define NS_KEY_HOME            ::Nous::Key::Home
#define NS_KEY_END             ::Nous::Key::End
#define NS_KEY_CAPS_LOCK       ::Nous::Key::CapsLock
#define NS_KEY_SCROLL_LOCK     ::Nous::Key::ScrollLock
#define NS_KEY_NUM_LOCK        ::Nous::Key::NumLock
#define NS_KEY_PRINT_SCREEN    ::Nous::Key::PrintScreen
#define NS_KEY_PAUSE           ::Nous::Key::Pause
#define NS_KEY_F1              ::Nous::Key::F1
#define NS_KEY_F2              ::Nous::Key::F2
#define NS_KEY_F3              ::Nous::Key::F3
#define NS_KEY_F4              ::Nous::Key::F4
#define NS_KEY_F5              ::Nous::Key::F5
#define NS_KEY_F6              ::Nous::Key::F6
#define NS_KEY_F7              ::Nous::Key::F7
#define NS_KEY_F8              ::Nous::Key::F8
#define NS_KEY_F9              ::Nous::Key::F9
#define NS_KEY_F10             ::Nous::Key::F10
#define NS_KEY_F11             ::Nous::Key::F11
#define NS_KEY_F12             ::Nous::Key::F12
#define NS_KEY_F13             ::Nous::Key::F13
#define NS_KEY_F14             ::Nous::Key::F14
#define NS_KEY_F15             ::Nous::Key::F15
#define NS_KEY_F16             ::Nous::Key::F16
#define NS_KEY_F17             ::Nous::Key::F17
#define NS_KEY_F18             ::Nous::Key::F18
#define NS_KEY_F19             ::Nous::Key::F19
#define NS_KEY_F20             ::Nous::Key::F20
#define NS_KEY_F21             ::Nous::Key::F21
#define NS_KEY_F22             ::Nous::Key::F22
#define NS_KEY_F23             ::Nous::Key::F23
#define NS_KEY_F24             ::Nous::Key::F24
#define NS_KEY_F25             ::Nous::Key::F25

/* Keypad */
#define NS_KEY_KP_0            ::Nous::Key::KP0
#define NS_KEY_KP_1            ::Nous::Key::KP1
#define NS_KEY_KP_2            ::Nous::Key::KP2
#define NS_KEY_KP_3            ::Nous::Key::KP3
#define NS_KEY_KP_4            ::Nous::Key::KP4
#define NS_KEY_KP_5            ::Nous::Key::KP5
#define NS_KEY_KP_6            ::Nous::Key::KP6
#define NS_KEY_KP_7            ::Nous::Key::KP7
#define NS_KEY_KP_8            ::Nous::Key::KP8
#define NS_KEY_KP_9            ::Nous::Key::KP9
#define NS_KEY_KP_DECIMAL      ::Nous::Key::KPDecimal
#define NS_KEY_KP_DIVIDE       ::Nous::Key::KPDivide
#define NS_KEY_KP_MULTIPLY     ::Nous::Key::KPMultiply
#define NS_KEY_KP_SUBTRACT     ::Nous::Key::KPSubtract
#define NS_KEY_KP_ADD          ::Nous::Key::KPAdd
#define NS_KEY_KP_ENTER        ::Nous::Key::KPEnter
#define NS_KEY_KP_EQUAL        ::Nous::Key::KPEqual

#define NS_KEY_LEFT_SHIFT      ::Nous::Key::LeftShift
#define NS_KEY_LEFT_CONTROL    ::Nous::Key::LeftControl
#define NS_KEY_LEFT_ALT        ::Nous::Key::LeftAlt
#define NS_KEY_LEFT_SUPER      ::Nous::Key::LeftSuper
#define NS_KEY_RIGHT_SHIFT     ::Nous::Key::RightShift
#define NS_KEY_RIGHT_CONTROL   ::Nous::Key::RightControl
#define NS_KEY_RIGHT_ALT       ::Nous::Key::RightAlt
#define NS_KEY_RIGHT_SUPER     ::Nous::Key::RightSuper
#define NS_KEY_MENU            ::Nous::Key::Menu
