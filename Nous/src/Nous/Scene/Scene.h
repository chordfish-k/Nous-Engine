#pragma once

#include "entt.hpp"

#include "Component.h"

#include "Nous/Core/Timestep.h"

namespace Nous {

    class Scene
    {
    public:
        Scene();
        ~Scene();

        entt::entity CreateEntity();

        // 暂时
        entt::registry& Reg() { return m_Registry; }

        void OnUpdate(Timestep dt);
    private:
        entt::registry m_Registry; //注册表=实体上下文，包含所有实体数据
    };
}

