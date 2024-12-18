﻿#pragma once

#include "Nous/Core/UUID.h"
#include "Nous/Scene/Scene.h"
#include "Nous/Scene/Component.h"

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
            if (HasComponent<T>())
            {
                NS_CORE_WARN("实体已存在组此件");
                return GetComponent<T>();
            }

            T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<T>(*this, component);
            return component;
        }

        template<typename T, typename... Args>
        T& AddOrReplaceComponent(Args&&... args)
        {
            T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<T>(*this, component);
            return component;
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
            m_Scene->OnComponentRemoved<T>(*this, GetComponent<T>());
            m_Scene->m_Registry.remove<T>(m_EntityHandle); // 只在实体拥有该组件时触发
        }

        operator bool() const { return m_EntityHandle != entt::null && m_Scene != nullptr; }
        operator entt::entity() const { return m_EntityHandle; }
        operator uint32_t() const { return (uint32_t)m_EntityHandle; }

        UUID GetUUID() { return GetComponent<CUuid>().ID; }
        std::string GetName() { return GetComponent<CTag>().Tag; }
        CTransform& GetTransform() { return GetComponent<CTransform>(); }

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

