#pragma once

#include "Nous/Asset/Asset.h"

namespace Nous {

    enum class EditorEventType
    {
        OpenScene = 0
    };

    struct EditorEvent
    {
        virtual EditorEventType GetEventType() const = 0;
        virtual const char* GetName() const = 0;
    };

#define EDITOR_EVENT_CLASS_TYPE(type) static EditorEventType GetStaticType() { return EditorEventType::type; } \
								virtual EditorEventType GetEventType() const override { return GetStaticType(); } \
								virtual const char* GetName() const override { return #type; }

    struct OpenSceneEvent : public EditorEvent
    {
        AssetHandle Handle;
        OpenSceneEvent(const AssetHandle& handle) : Handle(handle) {};

        EDITOR_EVENT_CLASS_TYPE(OpenScene);
    };

    class Observer;

    class EditorEventRepeater
    {
    public:
        static void AddObserver(Observer* observer);
        static void Emit(EditorEvent& e);

    private:
        static std::vector<Observer*> observers;
    };

    class EditorEventDispatcher
    {
        template<typename T>
        using EventFn = std::function<void(T&)>;
    public:
        EditorEventDispatcher(EditorEvent& event)
            :m_Event(event)
        {
        }

        template<typename T, typename F>
        void Dispatch(const F& func)
        {
            if (m_Event.GetEventType() == T::GetStaticType())
                func(static_cast<T&>(m_Event));
        }
    private:
        EditorEvent& m_Event;
    };

}
