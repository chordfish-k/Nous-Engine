#pragma once

#include "pch.h"

#include "Base.h"
#include "Nous/Event/Event.h"

namespace Nous {

    struct WindowProps
    {
        std::string Title;
        uint32_t Width;
        uint32_t Height;

        WindowProps(const std::string& title = "Nous",
                    unsigned int width = 1600,
                    unsigned int height = 900)
                : Title(title), Width(width), Height(height)
        {
        }
    };

    // 桌面窗口接口，给不同操作系统实现
    class NOUS_API Window
    {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        virtual ~Window() {}

        virtual void OnUpdate() = 0;

        virtual unsigned int GetWidth() const = 0;
        virtual unsigned int GetHeight() const = 0;

        // 窗口属性
        virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
        virtual void SetVSync(bool enable) = 0;
        virtual bool IsVSync() const = 0;

        virtual void* GetNativeWindow() const = 0;

        static Scope<Window> Create(const WindowProps& props = WindowProps());
    };

}
