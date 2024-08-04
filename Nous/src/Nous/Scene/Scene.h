#pragma once

#include "entt.hpp"

#include "Nous/Core/Timestep.h"

namespace Nous {

    class Entity;

    class Scene
    {
    public:
        Scene();
        ~Scene();

        Entity CreateEntity(const std::string& name = std::string());

        // 暂时
        entt::registry& Reg() { return m_Registry; }

        void OnUpdate(Timestep dt);
        void OnViewportResize(uint32_t width, uint32_t height);
    private:
        entt::registry m_Registry; //注册表=实体上下文，包含所有实体数据
        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

        friend class Entity;
        friend class SceneHierarchyPanel;
    };
}

