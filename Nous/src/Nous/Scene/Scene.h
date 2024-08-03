#pragma once

#include "entt.hpp"

namespace Nous {

    class Scene
    {
    public:
        Scene();
        ~Scene();
    private:
        entt::registry m_Registry; //注册表=实体上下文，包含所有实体数据
    };

}

