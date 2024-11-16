#pragma once

#include <functional>

#include "Nous/Asset/Asset.h"

namespace Nous {

    enum class AppEventType
    {
        OpenScene = 0,
        ChangeRunningScene,
        AssetFileDoubleClick,
        SavePrefab,
        ConsoleClear
    };

    struct AppEvent
    {
        virtual AppEventType GetEventType() const = 0;
        virtual const char* GetName() const = 0;
    };

#define EDITOR_EVENT_CLASS_TYPE(type) static AppEventType GetStaticType() { return AppEventType::type; } \
								virtual AppEventType GetEventType() const override { return GetStaticType(); } \
								virtual const char* GetName() const override { return #type; }

    // EVENT
    struct OpenSceneEvent : public AppEvent
    {
        AssetHandle Handle;
        OpenSceneEvent(const AssetHandle& handle) : Handle(handle) {};

        EDITOR_EVENT_CLASS_TYPE(OpenScene);
    };
    
    struct ChangeRunningSceneEvent : public AppEvent
    {
        AssetHandle Handle;
        ChangeRunningSceneEvent(const AssetHandle& handle) : Handle(handle) {};

        EDITOR_EVENT_CLASS_TYPE(ChangeRunningScene);
    };
    struct ConsoleClearEvent : public AppEvent
    {
        ConsoleClearEvent() {}

        EDITOR_EVENT_CLASS_TYPE(ConsoleClear);
    };

    struct SavePrefabEvent : public AppEvent
    {
        UUID Root;
        std::filesystem::path Dir;
        SavePrefabEvent(const UUID& root, const std::filesystem::path& dir) : Root(root), Dir(dir) {};

        EDITOR_EVENT_CLASS_TYPE(SavePrefab);
    };

    struct AssetFileDoubleClickEvent : public AppEvent
    {
        AssetHandle Handle;
        AssetFileDoubleClickEvent(const AssetHandle& handle) : Handle(handle) {};

        EDITOR_EVENT_CLASS_TYPE(AssetFileDoubleClick);
    };
    // EVENT

    class Observer;

    class AppEventEmitter
    {
    public:
        static void AddObserver(Observer* observer);
        static void Emit(AppEvent& e);

    private:
        static std::vector<Observer*> observers;
    };

    class AppEventDispatcher
    {
        template<typename T>
        using EventFn = std::function<void(T&)>;
    public:
        AppEventDispatcher(AppEvent& event)
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
        AppEvent& m_Event;
    };

}
