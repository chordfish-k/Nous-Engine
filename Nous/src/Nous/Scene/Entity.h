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
            NS_CORE_ASSERT(HasComponent<T>(), "实体没有此组件！");
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template<typename T>
        bool HasComponent()
        {
            return m_Scene->m_Registry.has<T>(m_EntityHandle);
        }

        template<typename T>
        void RemoveComponent()
        {
            NS_CORE_ASSERT(HasComponent<T>(), "实体没有此组件！");
            m_Scene->m_Registry.remove<T>(m_EntityHandle); // 只在实体拥有该组件时触发
        }

        operator bool() const { return m_EntityHandle != entt::null; }
        operator uint32_t() const { return (uint32_t)m_EntityHandle; }
        bool operator==(const Entity& other) const {
            return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
        }
        bool operator!=(const Entity& other) const {
            return !(*this == other);
        }
    private:
        entt::entity m_EntityHandle{ entt::null };
        // 暂时用原生指针
        Scene* m_Scene = nullptr;
    };
}

