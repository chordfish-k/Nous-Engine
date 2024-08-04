#pragma once

#include "Scene.h"

#include "entt.hpp"

namespace Nous {

    class Entity
    {
    public:
        Entity() = default;
        Entity(entt::entity handle, Scene* scene);
        Entity(const Entity& other) = default;

        template<class T, typename ... Args>
        T& AddComponent(Args&&... args)
        {
            NS_CORE_ASSERT(!HasComponent<T>(), "实体已有此组件！");
            return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
        }

        template<typename T>
        T& GetComponent()
        {
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template<class T>
        bool HasComponent()
        {
            return m_Scene->m_Registry.any_of<T>(m_EntityHandle);
        }

        template<typename T>
        void RemoveComponent()
        {
            NS_CORE_ASSERT(HasComponent<T>(), "实体没有此组件！");
            m_Scene->m_Registry.remove<T>(m_EntityHandle);
        }

        operator bool() const { return m_EntityHandle != entt::null; }
    private:
        entt::entity m_EntityHandle{ 0 };
        // 暂时用原生指针
        Scene* m_Scene = nullptr;
    };
}

