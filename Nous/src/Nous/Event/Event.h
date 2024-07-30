#pragma once

#include "pch.h"
#include "Nous/Core/Base.h"

#include "spdlog/fmt/ostr.h"

namespace Nous {

    // 事件系统，目前是阻塞的

    enum class EventType
    {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
        AppTick, AppUpdate, AppRender,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
    };

    enum EventCategory
    {
        None = 0,
        EventCategoryApplication	= BIT(0),
        EventCategoryInput			= BIT(1),
        EventCategoryKeyboard		= BIT(2),
        EventCategoryMouse			= BIT(3),
        EventCategoryMouseButton	= BIT(4)
    };

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type; } \
								virtual EventType GetEventType() const override { return GetStaticType(); } \
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category)  virtual int GetCategoryFlags() const override { return category; }

    // 事件类
    class NOUS_API Event
    {
        friend class EventDispatcher;
    public:
        virtual ~Event() = default;

        bool Handled = false; // 是否已被处理

        virtual EventType GetEventType() const = 0;
        virtual const char* GetName() const = 0;
        virtual int GetCategoryFlags() const = 0;
        virtual std::string ToString() const { return GetName(); };

        bool IsInCategory(EventCategory category)
        {
            return GetCategoryFlags() & category;
        }
    };

    // 事件分发器
    class EventDispatcher
    {
        template<typename T>
        using EventFn = std::function<bool(T&)>;
    public:
        EventDispatcher(Event& event)
                :m_Event(event)
        {
        }

        template<typename T, typename F>
        bool Dispatch(const F& func)
        {
            // 判断事件类型是否合法
            if (m_Event.GetEventType() == T::GetStaticType())
            {
                // 处理事件，返回成功与否
                m_Event.Handled |= func(static_cast<T&>(m_Event));
                return true;
            }
            return false;
        }
    private:
        Event& m_Event;
    };

    inline std::ostream& operator<<(std::ostream& os, const Event& e)
    {
        return os << e.ToString();
    }
}
