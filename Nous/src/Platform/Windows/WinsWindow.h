﻿#pragma once

#include "Nous/Core/Window.h"
#include "Nous/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>

namespace Nous {

    class WinsWindow : public Window
    {
    public:
        WinsWindow(const WindowProps &props);

        virtual ~WinsWindow();

        void OnUpdate() override;

        unsigned int GetWidth() const override { return m_Data.Width; }

        unsigned int GetHeight() const override { return m_Data.Height; }

        // Window 属性
        void SetEventCallback(const EventCallbackFn &callback) override { m_Data.EventCallback = callback; }

        void SetVSync(bool enabled) override;

        bool IsVSync() const override;

        virtual void *GetNativeWindow() const override { return m_Window; };

    private:
        virtual void Init(const WindowProps &props);

        virtual void Shutdown();

    private:
        GLFWwindow *m_Window;
        GraphicsContext *m_Context;

        struct WindowData
        {
            std::string Title;
            unsigned int Width, Height;
            bool VSync;

            EventCallbackFn EventCallback;
        };
        WindowData m_Data;
    };

}

